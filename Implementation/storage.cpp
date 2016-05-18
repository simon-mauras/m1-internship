#ifndef _INCLUDE_STORAGE
#define _INCLUDE_STORAGE

#include <cstddef>

class Storage
{
  public:
    Storage() {};
    virtual ~Storage() {}
    virtual void read(char* buff, size_t pos, size_t count) = 0;
    virtual void write(char* buff, size_t pos, size_t count) = 0;
};

#endif
