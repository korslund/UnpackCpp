#include "base/unpack_base.hpp"
#include "common.cpp"

// A dummy "unpacker" that just writes some random files
struct DummyUnpack : UnpackBase
{
  void unpack(StreamPtr input,
              StreamFactoryPtr out,
              Progress *prog = NULL,
              const FileList *list = NULL)
  {
    StreamPtr s;

    out->open("test1.dummy.empty");
    s = out->open("test2.txt");
    assert(s);
    s->write("Hello dolly\n", 12);
    s.reset();
    s = out->open("dir1/");
    //assert(!s);
    s = out->open("dir2/test3.txt");
    s->write("This is Louis, Dolly\n", 21);
  }

  using UnpackBase::unpack;
};

int main()
{
  cout << "Unpacking some fake files:\n";
  DummyUnpack dummy;

  dummy.unpack(StreamPtr(), makeFact());

  cout << "Done\n";
  return 0;
}
