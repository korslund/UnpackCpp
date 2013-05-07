#include "auto.hpp"
#include "common.cpp"

using namespace std;
using namespace UnpackCpp;

void test(const std::string &file)
{
  cout << "Unpacking "+file+":\n  ";
  AutoUnpack unp;
  try
    {
      unp.unpack(file, makeFact());
    }
  catch(std::exception &e)
    {
      cout << e.what() << endl;
      return;
    }
  cout << "OK\n";
}

int main()
{
  test("test.sh");
  test("doesn't exist");
  test("archives/test.zip");
  test("archives/test.rar");
  test("archives/test.tar");
  test("archives/test.tar.gz");
  test("archives/test.tar.bz2");
  return 0;
}
