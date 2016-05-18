#ifndef _INCLUDE_STORAGE_FSTREAM
#define _INCLUDE_STORAGE_FSTREAM

#include "storage.cpp"
#include <stdexcept>
#include <cstdio>

using namespace std;

class Storage_cstdio: public Storage
{
  public:
    Storage_cstdio(const char* filename);
    ~Storage_cstdio();
    void read(char* buff, size_t pos, size_t count);
    void write(char* buff, size_t pos, size_t count);
  
  protected:
    FILE* f;
};

Storage_cstdio::Storage_cstdio(const char* filename): Storage()
{
  f = fopen(filename, "r+");
  if (f == nullptr) {
    throw runtime_error("Cstdio error: open");
  }
}

Storage_cstdio::~Storage_cstdio()
{
  if (fclose(f) != 0) {
    throw runtime_error("Cstdio error: close");
  }
}

void Storage_cstdio::read(char* buff, size_t pos, size_t count)
{
  fseek (f, 0, SEEK_END);
  size_t length = ftell(f);
  if (length < pos+count)
  {
    char* fill = new char[pos+count-length]();
    fwrite(fill, sizeof(char), pos+count-length, f);
    delete[] fill;
  }
  fseek(f, pos, SEEK_SET);
  fread(buff, sizeof(char), count, f);
  if (ferror(f)) {
    throw runtime_error("Cstdio error: read");
  }
}

void Storage_cstdio::write(char* buff, size_t pos, size_t count)
{
  fseek(f, pos, SEEK_SET);
  fwrite(buff, sizeof(char), count, f);
  fflush(f);
  if (ferror(f)) {
    throw runtime_error("Cstdio error: write");
  }
}

#endif
