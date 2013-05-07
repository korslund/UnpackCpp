#include "zipfile.hpp"
#include "misc/fileparser.hpp"
#include <stdexcept>
#include <zlib.h>
#include <assert.h>
#include <memory.h>

using namespace UnpackCpp;
using namespace Mangle::Stream;

struct Entry
{
  bool isComp;
  uint32_t offset;
};

struct ZipFile::_Int
{
  FileParser input;
  std::vector<Entry> entries;
};

const uint32_t FooterMagic = 0x06054b50;
const uint32_t FooterMagic64 = 0x06064b50;
const uint32_t EntryMagic = 0x02014b50;

static void fail(const std::string &msg)
{
  throw std::runtime_error("ZIP error: " + msg);
}

struct FooterInfo
{
  uint32_t entries, rootSize, rootPos;
};

/* Search from the end of the stream for the footer data
   structure. Throws if not found.
 */
static void findFooter(StreamPtr inp, FooterInfo &out)
{
  /* We assume the footer is to be found within the last 64K of the
     file end. If it isn't, we fail.
  */
  const uint64_t MAX = 64*1024;

  // Figure out search window size and position
  uint64_t fpos = inp->size();
  uint64_t size = MAX;
  if(fpos < size) size = fpos;
  fpos -= size;

  // Use direct pointer access if possible, otherwise copy data into a
  // buffer
  std::vector<char> buf;
  const char *search = NULL;
  if(inp->hasPtr)
    search = (char*)inp->getPtr(fpos, size);
  else
    {
      buf.resize(size);
      inp->seek(fpos);
      inp->read(&buf[0], buf.size());
      search = &buf[0];
    }

  // Search backwards from the end
  for(int left = 20; left <= size; left++)
    {
      const char *cur = search + (size - left);
      if(*cur != 'P') continue;

      const uint32_t *head = (const uint32_t*)cur;
      if(*head == FooterMagic)
        {
          head++;
          if(*head++ != 0) fail("Multi-disk archives not supported");
          const uint16_t* p16 = (const uint16_t*)head++;
          out.entries = *p16++;
          if(*p16 != out.entries) fail("Invalid footer data");
          out.rootSize = *head++;
          out.rootPos = *head;
          return;
        }
      else if(*head == FooterMagic64 && left >= 56)
        {
          /* This code has never actually been tested, as I haven't
             yet found any 64 bit archives in the wild. Since we don't
             want live untested code, disable it.
           */
          fail("Zip64 footer found - not supported yet.");
          /*
          head += 4;
          if(*head++ || *head++)
            fail("Multi-disk archives not supported");
          if(*head++) fail("Footer data too large (expected 32 bit value)");
          out.entries = *head++;
          if(*head++ || *head++ != out.entries)
            fail("Invalid footer data");
          if(*head++) fail("Footer data too large (expected 32 bit value)");
          out.rootSize = *head++;
          if(*head++) fail("Footer data too large (expected 32 bit value)");
          out.rootPos = *head;
          return;
          */
        }
    }

  fail("Unable to find footer data");
}

static void parseDir(FileParser &inp, const FooterInfo &foot,
                     std::vector<ZipFile::File> &files,
                     std::vector<Entry> &entries)
{
  if(foot.rootPos + foot.rootSize > inp.size())
    fail("Corrupt directory table specification");

  inp.seek(foot.rootPos);
  uint32_t left = foot.rootSize;
  if(foot.entries > 1024*1024)
    fail("Entry number is too large - assuming file is corrupted");

  files.resize(foot.entries);
  entries.resize(foot.entries);

  for(uint32_t i = 0; i<foot.entries; i++)
    {
      ZipFile::File &f = files[i];
      Entry &e = entries[i];

      if(inp.getUint() != EntryMagic)
        fail("Corrupt directory entry");
      inp.skip(6);
      {
        int comp = inp.getUshort();
        e.isComp = comp != 0;
        if(comp != 0 && comp != 8)
          fail("Unknown compression method");
      }
      inp.skip(8);
      f.comp = inp.getUint();
      f.size = inp.getUint();

      if(!e.isComp && f.comp != f.size)
        fail("Entry size mismatch");

      int name = inp.getUshort();
      int skip = inp.getUshort() + inp.getUshort();
      if(inp.getUshort()) fail("Disk number was not zero");
      inp.skip(6);
      e.offset = inp.getUint();

      if(inp.tell() + name + skip > foot.rootPos+foot.rootSize)
        fail("Directory entry out of bounds");
      f.name = inp.readString(name);
      inp.skip(skip);
    }
}

void ZipFile::openZipArchive(StreamPtr input)
{
  ptr.reset(new _Int);
  ptr->input.setStream(input);
  assert(input->isReadable);
  assert(input->isSeekable);
  assert(input->hasPosition);
  assert(input->hasSize);

  FooterInfo foot;
  findFooter(input, foot);
  parseDir(ptr->input, foot, files, ptr->entries);
  assert(files.size() == ptr->entries.size());
}

void ZipFile::unpackFile(int index, StreamPtr output)
{
  assert(index >= 0 && index < files.size());
  assert(files.size() == ptr->entries.size());
  assert(output->isWritable);

  const Entry &e = ptr->entries[index];
  const File &f = files[index];
  FileParser &inp = ptr->input;

  // Skip header
  inp.seek(e.offset);
  inp.skip(26);
  int toskip = inp.getUshort() + inp.getUshort();
  inp.skip(toskip);

  // Tell the stream how many bytes we are going to write
  output->reserve(f.size);

  if(!e.isComp)
    {
      // Stored files
      assert(f.size == f.comp);
      inp.store(output, f.size);
      return;
    }

  // Compressed files
  const uint32_t BUFSIZE = 32*1024;
  z_stream strm;
  unsigned char in[BUFSIZE];
  unsigned char out[BUFSIZE];

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  if(inflateInit2(&strm, -MAX_WBITS) != Z_OK)
    fail("Failed to initialize zlib");

  int ret = Z_OK;
  uint32_t left = f.comp, written = 0;
  try
    {
      strm.avail_in = 0;
      strm.next_in = in;

      while(true)
        {
          // Move unused input data to the beginning of the buffer
          if(strm.avail_in)
            {
              // We'll only get here if inflate() ran out of usable
              // input data
              assert(ret == Z_BUF_ERROR);
              memmove(in, strm.next_in, strm.avail_in);
            }

          // Calculate where and how much to read
          unsigned char *readPtr = in+strm.avail_in;
          uint32_t readCount = BUFSIZE-strm.avail_in;

          // Cap read size to available data
          if(readCount > left) readCount = left;
          assert(readCount+readPtr <= in+BUFSIZE);
          if(readCount)
            {
              inp.read(readPtr, readCount);
              assert(left >= readCount);
              left -= readCount;
            }

          // Calculate how much total input we have now
          strm.avail_in = readCount + strm.avail_in;
          strm.next_in = in;
          assert(strm.avail_in <= BUFSIZE);

          /* Loop the unpacker and keep dumping data until inflate()
             starts to complain.
           */
          do
            {
              // Set up output buffer and inflate
              strm.avail_out = BUFSIZE;
              strm.next_out = out;
              ret = inflate(&strm, Z_NO_FLUSH);

              if(ret == Z_STREAM_END || ret == Z_OK)
                {
                  // Get rid of all written data
                  uint32_t writeCount = BUFSIZE - strm.avail_out;
                  if(writeCount)
                    {
                      output->write(out, writeCount);
                      written += writeCount;
                    }
                }
              else if(ret != Z_BUF_ERROR)
                fail("Failed to decompress data");
            }
          while(ret == Z_OK);

          if(ret == Z_STREAM_END || written >= f.size)
            break;
        }
    }
  catch(...) { inflateEnd(&strm); throw; }
  inflateEnd(&strm);
  if(ret != Z_STREAM_END || left != 0 || written != f.size)
    fail("Error decompressing data - size mismatch");
}
