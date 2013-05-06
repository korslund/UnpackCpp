#include "unpack_zip.hpp"
#include <iostream>
#include <assert.h>

#include "print_dir.hpp"

using namespace std;
using namespace Unpack;

int main()
{
  string file = "archives/test.zip";

  cout << "Unpacking "+file+":\n";
  UnpackZip zip;
  zip.unpack(file, "_outdir2");

  printDir("_outdir2");
  cout << "Done\n";
  return 0;
}
