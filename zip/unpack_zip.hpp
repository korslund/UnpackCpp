#ifndef __UNPACK_ZIP_HPP
#define __UNPACK_ZIP_HPP

#include "../base/base.hpp"

namespace UnpackCpp
{
  /* ZIP file unpacker.

     This implementation uses the ZZIP library.
   */
  struct UnpackZip : UnpackBase
  {
    void unpack(const std::string &file,
                Mangle::VFS::StreamFactoryPtr output,
                Progress *prog = NULL,
                const FileList *list = NULL);

    using UnpackBase::unpack;
  };
}
#endif
