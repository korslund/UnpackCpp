#ifndef __UNPACKCPP_UNPACK_BASE_HPP
#define __UNPACKCPP_UNPACK_BASE_HPP

#include <mangle/vfs/stream_factory.hpp>
#include <set>

#include "progress.hpp"

namespace UnpackCpp
{
  /*
    Base class for all unpackers. Only the first unpack() function is
    implemented in subclasses, the others are convenient wrappers
    around it.

    The unpack() function takes an input Mangle::Stream::Stream, an
    output Mangle::VFS::StreamFactory, an optional progress updater
    functor, and an optional list of output files to produce.

    How the VFS::StreamFactory is used:

    - open("filename") is called for all files in the archive. The
      returned stream is expected to be writable, except for directory
      entries. The data for each file is written to the stream, and
      the StreamPtr is reset.

    - Returning an empty StreamPtr is allowed, and will cause the file
      to be skipped. This can be used to index the archive.

    - Filenames may include paths. The reciever is expected to create
      necessary parent directories.

    - Directories (paths ending with a slash) may be sent to
      open(). The StreamFactory is expected to create empty
      directories in those cases, and to return an empty StreamPtr.
      Both forward and backward slashes should be expected.

    - The StreamFactory itself is responsible for throwing exceptions
      on writing errors. The unpacker will only throw on unpacking
      errors.

    - Instead of using StreamFactory directly, you may use the
      directory version to write to a file system directory.

    About the FileList list:

    - If the 'list' parameter is specified, only the files listed
      there will be unpacked. Files in the list which do not match any
      in the archive, will cause an error.

      Names must match exactly with the files in the archive,
      including case and slash type. For that reason, it is recommened
      to ONLY use filenames that have previously been generated with
      the same unpacker.

    - If the list is missing or empty, all files are unpacked.

    - Implementations may choose to behave differently depending on
      whether the list is present or not. Some future EXE unpackers
      for example may enter a slow "explore mode" when a list is
      missing, but will be in a much faster "unpack mode" when given
      the right data. In those cases, the list names may contain
      additional meta-data that do not really represent actual
      filenames.
   */
  struct UnpackBase
  {
    typedef std::set<std::string> FileList;

    // Unpack the given archive file into the stream factory.
    virtual void unpack(Mangle::Stream::StreamPtr input,
                        Mangle::VFS::StreamFactoryPtr output,
                        Progress *prog = NULL,
                        const FileList *list = NULL) = 0;

    // File reader version
    void unpack(const std::string &file,
                Mangle::VFS::StreamFactoryPtr output,
                Progress *prog = NULL,
                const FileList *list = NULL);

    // Directory writer versions
    void unpack(Mangle::Stream::StreamPtr input, const std::string &dir,
                Progress *prog = NULL,
                const FileList *list = NULL);
    void unpack(const std::string &file, const std::string &dir,
                Progress *prog = NULL,
                const FileList *list = NULL);

    /* Generate an index of an archive. This is the same as running
       unpack() and gathering up the names generated through the
       StreamFactory.

       An optional StreamFactoryPtr may be provided to accept output
       data. If it is empty, no data is extracted.
    */
    void index(Mangle::Stream::StreamPtr input, FileList &output,
               Progress *prog = NULL,
               Mangle::VFS::StreamFactoryPtr write =
               Mangle::VFS::StreamFactoryPtr());
    void index(const std::string &file, FileList &output,
               Progress *prog = NULL,
               Mangle::VFS::StreamFactoryPtr write =
               Mangle::VFS::StreamFactoryPtr());
  };
}
#endif
