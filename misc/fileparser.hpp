#ifndef __UNP_FILE_PARSER_HPP_
#define __UNP_FILE_PARSER_HPP_

#include <mangle/stream/filters/pure_filter.hpp>
#include <stdexcept>
#include <stdint.h>
#include <vector>

namespace UnpackCpp
{
  namespace MS = Mangle::Stream;

  struct FileParser : MS::PureFilter
  {
    FileParser() {}
    FileParser(MS::StreamPtr _src)
      : MS::PureFilter(_src) {}

    template <typename T>
    void readT(T &t)
    {
      // TODO: Implement endian byte swapping
      if(read(&t, sizeof(t)) != sizeof(t))
        throw std::runtime_error("Read error or end of stream");
    }

    template <typename T>
    T getT()
    {
      T t;
      readT(t);
      return t;
    }

    int32_t getInt() { return getT<int32_t>(); }
    uint32_t getUint() { return getT<uint32_t>(); }
    uint8_t getByte() { return getT<uint8_t>(); }
    uint16_t getUshort() { return getT<uint16_t>(); }

    uint8_t bitbuf[3];
    int bitnum;

    void startBits()
    {
      read(bitbuf, 3);
      bitnum = 0;
    }

    void moveBits(int i)
    {
      bitnum += i;
      if(bitnum >= 8)
        {
          bitbuf[0] = bitbuf[1];
          bitbuf[1] = bitbuf[2];
          bitbuf[2] = getByte();
          bitnum -= 8;
        }
      assert(bitnum < 8);
    }

    uint16_t getBits()
    {
      return (bitbuf[0] >> bitnum) |
             (bitbuf[1] << (8-bitnum)) |
             (bitbuf[2] << (16-bitnum));
    }

    void skip(uint32_t bytes)
    {
      if(!bytes) return;
      if(isSeekable && hasPosition)
        seek(tell() + bytes);
      else
        {
          char buf[1024];
          while(bytes && !eof())
            {
              int count = 1024;
              if(bytes < count) count = bytes;
              read(buf,count);
              bytes -= count;
            }
        }
    }

    std::string readString(int size)
    {
      std::vector<char> buf(size);
      read(&buf[0],size);
      return std::string(&buf[0],size);
    }

    void store(MS::StreamPtr out, uint32_t bytes)
    {
      char buf[1024];
      while(bytes && !eof())
        {
          int count = 1024;
          if(bytes < count) count = bytes;
          read(buf,count);
          out->write(buf,count);
          bytes -= count;
        }
    }
  };
}
#endif
