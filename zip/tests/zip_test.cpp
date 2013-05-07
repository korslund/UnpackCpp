#include "zip/unpack_zip.hpp"
#include "base/tests/common.cpp"

using namespace std;
using namespace UnpackCpp;

int main()
{
  string file = "test.zip";

  cout << "Unpacking "+file+":\n";
  UnpackZip zip;
  zip.unpack(file, makeFact());

  cout << "Done\n";
  return 0;
}
