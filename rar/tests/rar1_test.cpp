#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <stdexcept>
#include <vector>

#include "misc/fileparser.hpp"

using namespace std;
using namespace UnpackCpp;

#pragma pack(push, 1)
struct Base // 7
{
  uint16_t crc;
  uint8_t type;
  uint16_t flags, size;
};

struct FileHeader
{
  uint32_t packed, unpacked;
  char os;
  int crc, time;
  char unpackVer, unpackMethod;
  uint16_t nameLen;
  uint32_t fileAttr;
};
#pragma pack(pop)

enum HeaderType
  {
    HT_MainHeader = 0x73,
    HT_FileHeader = 0x74,
    HT_CommentHeader = 0x75,
    HT_EndHeader = 0x7b
  };

enum HeaderFlags
  {
    MHF_Volume          = 0x001,
    MHF_Lock            = 0x004,
    MHF_Solid           = 0x008,
    MHF_Password        = 0x080,
    MHF_FirstVolume     = 0x100,
    MHF_EncryptVer      = 0x200,

    FHF_Split           = 0x003,
    FHF_Directory       = 0x0e0,
    FHF_Large           = 0x100,
    FHF_Unicode         = 0x200,
    FHF_Salt            = 0x400,

    EHF_NextVolume      = 0x001,

    HF_LongBlock        = 0x8000
  };

enum UnpackMethod
  {
    UM_Store            = 48,
    UM_Unpack49         = 49,
    UM_Unpack51         = 51
  };

void fail(const std::string &msg)
{
  throw std::runtime_error("RAR unpack failed: " + msg);
}

void addHighBits(int64_t &v, int32_t high)
{
  int32_t *p = (int32_t*)&v;
  *(p+1) = high;
}

int c2i(char c) { return c; }

void unpack29(FileParser &parse, uint32_t size)
{
  parse.startBits();

  // TODO: Check for the Solid flag, should be in the file block
  // header.

  uint16_t i = parse.getBits();
  cout << "i=" << i << endl;

  if(i & 0x8000)
    {
      cout << "Block_PPM\n";

      
    }
  else
    cout << "Block_LZ\n";
}

struct UnRar
{
  FileParser parse;

  UnRar(const std::string &file)
    : parse(file)
  {
    int i = parse.getInt();

    if(i == 0x21726152 && parse.getByte() == 0x1a &&
       parse.getByte() == 0x07 && parse.getByte() == 0x00)
      readHeaders();

    else if(i == 0x5e7e4552)
      {
        cout << "Old format magic number found\n";
        // They seek back to 0 in this case, I suspect because they've
        // read 7 bytes instead of 4 at this point.

        /* We haven't implemented the old format type yet, although we
           should. We could use a test file.
        */
      }

    else
      cout << "Unrecognized RAR file\n";
  }

  void readHeaders()
  {
    string outf = "a.dat";

    while(!parse.eof())
      {
        uint32_t endPos = parse.tell();
        Base base;
        parse.readT(base);
        endPos += base.size;

        if(base.type == HT_MainHeader)
          {
            // Check the flags for unsupported options
            if(base.flags & (MHF_Password | MHF_EncryptVer) != 0)
              fail("Encrypted archives not supported");

            // Skip the rest of the header
            parse.seek(endPos);
          }
        else if(base.type == HT_FileHeader)
          {
            FileHeader fh;
            parse.readT(fh);

            if(base.flags & FHF_Split)
              fail("Split archives not supported");

            if(base.flags & FHF_Salt)
              fail("Encrypted files not supported");

            int64_t packSize = fh.packed;
            int64_t unpackSize = fh.unpacked;
            if(base.flags & FHF_Large)
              {
                // Support file sizes over 4Gb
                addHighBits(packSize, parse.getUint());
                addHighBits(unpackSize, parse.getUint());
              }
            else
              {
                // A value of 0xffffffff without the Large flag set is
                // used to indicate that the unpack size is unknown.
                if(unpackSize == 0xffffffff)
                  unpackSize = -1;
              }

            // Parse filename
            std::string filename;
            {
              if(fh.nameLen > 1000)
                fail("Filename too long");
              std::vector<char> buf(fh.nameLen);
              parse.read(&buf[0], fh.nameLen);
              filename.assign(&buf[0], fh.nameLen);
            }

            bool isDirectory = (base.flags & FHF_Directory) == FHF_Directory;

            // I think we should expect the filename to be UTF8 in any
            // case.
            if(base.flags & FHF_Unicode)
              cout << "WARNING: Unicode filename\n";

            // Skip the rest of the header
            parse.seek(endPos);

            // The unpackMethod field specifies the compression level
            // 0-5, and in ASCII for some reason.
            if(fh.unpackMethod == '0')
              {
                if(!isDirectory)
                  {
                    // Stored (no compression)
                    //cout << "STORED: " << filename << endl;
                    parse.skip(packSize);
                  }
              }
            else if(fh.unpackMethod >= '1' && fh.unpackMethod <= '5')
              {
                // We don't care what the level was, we only need to
                // know that the data is compressed.
                if(fh.unpackVer != 29)
                  fail("Unknown version");

                //MS::StreamPtr p(new MS::OutFileStream("output.dat"));
                unpack29(parse, packSize);
                return;
                //parse.skip(packSize);
              }
            else
              fail("Unknown compression specification");

            /*
            if(isDirectory)
              cout << filename << " (DIR)\n";
            else
              cout << filename <<  " (" << unpackSize << " bytes)\n";
            */
          }
        else if(base.type == HT_EndHeader)
          {
            if(base.flags & EHF_NextVolume)
              fail("Split archives not supported");
            break;
          }
        else
          fail("Unknown record type");
      }
  }
};

int main(int argc, char **argv)
{
  if(argc != 2)
    cout << "Specify a RAR file\n";
  else
    {
      UnRar unrar(argv[1]);
    }

  return 0;
}
