#include "auto.hpp"
#include "zip/unpack_zip.hpp"

#include <stdexcept>
#include <fstream>

using namespace UnpackCpp;
using namespace std;

/* We could generalize this stuff further, and create a Mangle::VFS
   based unpacker system with complete directory reading, instead of
   the unpack-and-dump approach we are using now.

   This would also lend itself to a plugin-based auto-detecter, where
   you inserted handlers instead of having hard-coded autodetect
   code. That would allow the code to be used even where some
   libraries are not available, and also makes it easy for the user to
   add their own unpackers.

   This isn't a priority right now.
 */

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

void AutoUnpack::unpack(Mangle::Stream::StreamPtr input, Mangle::VFS::StreamFactoryPtr output, Progress *prog, const FileList *list)
{
  UnpackBase *unp = NULL;

  assert(input->isReadable);
  assert(input->isSeekable);
  assert(input->hasPosition);

  // Magic number test
  {
    input->seek(0);
    int magic = 0;
    input->read(&magic, 4);

    if(magic == 0x04034b50) unp = new UnpackZip;
    else if(magic == 0x21726152)
      fail("RAR not implemented yet");
    else if((magic & 0xffff) == 0x5a4d)
      fail("Cannot open EXE files yet");
  }

  if(!unp)
    fail("Not a known archive type");

  unp->unpack(input, output, prog, list);
  delete unp;
}
