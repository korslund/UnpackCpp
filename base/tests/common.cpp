#include <mangle/stream/servers/string_writer.hpp>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace UnpackCpp;
using namespace Mangle::Stream;
using namespace Mangle::VFS;

struct PrintStream : StringWriter
{
  std::string output;

  PrintStream() : StringWriter(output) {}
  ~PrintStream() { cout << "  " << output << endl; }
};

struct PrintFactory : StreamFactory
{
  StreamPtr open(const std::string &name)
  {
    cout << "FILE: " << name << endl;
    return StreamPtr(new PrintStream);
  }
};

StreamFactoryPtr makeFact() { return StreamFactoryPtr(new PrintFactory); }
