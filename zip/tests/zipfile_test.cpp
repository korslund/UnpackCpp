#include "zip/zipfile.hpp"
#include <mangle/stream/servers/file_stream.hpp>
#include <iostream>
#include "misc/dirwriter.hpp"

using namespace std;
using namespace UnpackCpp;
using namespace Mangle::Stream;

int main(int argc, char **argv)
{
  ZipFile zip;

  std::string file = "test.zip";
  std::string where = "";

  if(argc >= 2) file = argv[1];
  if(argc >= 3) where = argv[2];

  cout << "Reading " << file << endl;
  try
    {
      zip.openZipArchive(FileStream::Open(file));

      if(where != "")
        {
          DirWriter dir(where);

          cout << "  Unpacking to " << where << ":\n";
          for(int i=0; i<zip.files.size(); i++)
            {
              const ZipFile::File &f = zip.files[i];
              cout << "    " << f.name << "   " << f.comp << "=>" << f.size;
              StreamPtr out = dir.open(f.name);
              if(out) zip.unpackFile(i, out);
              else cout << " (directory)";
              cout << endl;
            }
        }
    }
  catch(exception &e)
    {
      cout << "\nERROR: " << e.what() << endl;
    }
  return 0;
}
