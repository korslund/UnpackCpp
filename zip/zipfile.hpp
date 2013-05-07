#ifndef __UNPACK_ZIPFILE_HPP_
#define __UNPACK_ZIPFILE_HPP_

/* ZipFile is an untility class responsible for doing all the
   format-specific dirty work of zip file unpacking.
 */

#include <mangle/stream/stream.hpp>
#include <vector>
#include <stdint.h>

namespace UnpackCpp
{
  struct ZipFile
  {
    struct File
    {
      std::string name;
      uint64_t comp, size; // Compressed and uncompressed file size
    };

    std::vector<File> files;

    /* Parses a zip archive stream and fills in the 'files' list. The
       stream must be readable and seekable.
     */
    void openZipArchive(Mangle::Stream::StreamPtr input);

    /* Unpack a file from the 'files' list and write the data into the
       given stream.
     */
    void unpackFile(int index, Mangle::Stream::StreamPtr output);

  private:
    struct _Int;
    boost::shared_ptr<_Int> ptr;
  };
}

#endif
