#ifndef __UNPACK_ZIP_HPP
#define __UNPACK_ZIP_HPP

#include "../base/unpack_base.hpp"

namespace UnpackCpp
{
  /* ZIP file unpacker.
   */
  struct UnpackZip : UnpackBase
  {
    void unpack(Mangle::Stream::StreamPtr input,
                Mangle::VFS::StreamFactoryPtr output,
                Progress *prog = NULL,
                const FileList *list = NULL);

    using UnpackBase::unpack;
  };
}
#endif
