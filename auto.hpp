#ifndef __UNPACKCPP_AUTO_HPP
#define __UNPACKCPP_AUTO_HPP

#include "base/unpack_base.hpp"

namespace UnpackCpp
{
  /* An unpacker that auto-detects archive format.

     Currently supported formats:
     - ZIP (zip/unpack_zip.hpp)

     Throws an error if the file is missing, or if no supported format
     was detected.

     NOTE: some formats (especially EXE unpackers) will have wildly
     different performance statistics depending on whether or not a
     FileList is included.

     It is therefore recommended that you pre-generate a FileList
     using index(), and pass it along to unpackers if possible.
   */
  struct AutoUnpack : UnpackBase
  {
    void unpack(Mangle::Stream::StreamPtr input,
                Mangle::VFS::StreamFactoryPtr output,
                Progress *prog = NULL,
                const FileList *list = NULL);

    using UnpackBase::unpack;
  };
}
#endif
