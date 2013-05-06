#include "unpack_base.hpp"

#include "misc/dirwriter.hpp"
#include <mangle/stream/servers/file_stream.hpp>

using namespace Mangle::VFS;
using namespace Mangle::Stream;
using namespace UnpackCpp;

void UnpackBase::unpack(const std::string &file,
                        Mangle::VFS::StreamFactoryPtr output,
                        Progress *prog,
                        const FileList *list)
{ unpack(FileStream::Open(file),output,prog,list); }

void UnpackBase::unpack(Mangle::Stream::StreamPtr input, const std::string &dir,
                        Progress *prog,
                        const FileList *list)
{ unpack(input, StreamFactoryPtr(new UnpackCpp::DirWriter(dir)), prog, list); }

void UnpackBase::unpack(const std::string &file, const std::string &dir,
                        Progress *prog,
                        const FileList *list)
{ unpack(FileStream::Open(file),dir,prog,list); }

struct ListMaker : StreamFactory
{
  UnpackBase::FileList *list;
  StreamFactoryPtr redir;

  StreamPtr open(const std::string &name)
  {
    list->insert(name);
    if(redir)
      return redir->open(name);
    return StreamPtr();
  }
};

void UnpackBase::index(const std::string &file,
                       FileList &output,
                       Progress *prog,
                       StreamFactoryPtr write)
{ index(FileStream::Open(file),output,prog,write); }

void UnpackBase::index(StreamPtr file,
                       FileList &output,
                       Progress *prog,
                       StreamFactoryPtr write)
{
  ListMaker *m = new ListMaker;
  m->list = &output;
  m->redir = write;
  StreamFactoryPtr mp(m);

  // This basically unpacks the file normally, and pumps all the data
  // through ListMaker. The ListMaker makes note of all the filenames
  // given, and store them in 'output'.
  unpack(file, mp, prog);
}
