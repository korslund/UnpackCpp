#include "unpack_zip.hpp"

#include "zipfile.hpp"
#include <stdexcept>

using namespace UnpackCpp;
using namespace Mangle::Stream;

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

void UnpackZip::unpack(StreamPtr input, Mangle::VFS::StreamFactoryPtr output,
                       Progress *prog, const FileList *list)
{
  assert(input);
  assert(output);

  ZipFile zip;

  zip.openZipArchive(input);

  // Count up total size to unpack
  int64_t total = 0, current = 0;
  std::vector<int> dir;
  for(int i=0; i<zip.files.size(); i++)
  {
    const ZipFile::File &f = zip.files[i];

    // Is this file on the extract list?
    if(list && list->size())
      if(list->count(f.name) == 0)
        // Nope. Skip it.
        continue;

    dir.push_back(i);
    total += f.size;
  }

  if(list && (list->size() > dir.size()))
    fail("Missing requested files in archive");

  bool abort = false;

  // Update progress and check for abort status
  if(prog)
    abort = !prog->progress(total, current);

  for(int i=0; i<dir.size(); i++)
    {
      if(abort)
        break;

      const ZipFile::File &f = zip.files[dir[i]];

      // Fetch a writable stream
      StreamPtr outs = output->open(f.name);
      if(!outs) continue; // Skip directories
      assert(outs->isWritable);

      // Unpack the file
      zip.unpackFile(dir[i], outs);
      current += f.size;

      // Update progress and check for abort status
      if(prog)
        abort = !prog->progress(total, current);
    }
}
