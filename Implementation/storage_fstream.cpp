#ifndef _INCLUDE_STORAGE_FSTREAM
#define _INCLUDE_STORAGE_FSTREAM

#include "storage.cpp"
#include <stdexcept>
#include <fstream>

using namespace std;

class Storage_fstream: public Storage
{
  public:
    Storage_fstream(const char* filename);
    ~Storage_fstream();
    void read(char* buff, size_t pos, size_t count);
    void write(char* buff, size_t pos, size_t count);
  
  protected:
    fstream fs;
};

Storage_fstream::Storage_fstream(const char* filename): Storage()
{
  fs.open(filename, fstream::in | fstream::out | fstream::binary | fstream::trunc);
  if (!fs.good()) {
    throw runtime_error("Fstream error: open");
  }
}

Storage_fstream::~Storage_fstream()
{
  fs.close();
  if (!fs.good()) {
    throw runtime_error("Fstream error: close");
  }
}

void Storage_fstream::read(char* buff, size_t pos, size_t count)
{
  fs.seekg (0, fs.end);
  size_t length = fs.tellg();
  if (length < pos+count)
  {
    char* fill = new char[pos+count-length]();
    fs.write(fill, pos+count-length);
    delete[] fill;
  }
  fs.seekg(pos);
  fs.read(buff, count);
  if (!fs.good()) {
    throw runtime_error("Fstream error: read");
  }
}

void Storage_fstream::write(char* buff, size_t pos, size_t count)
{
  fs.seekp(pos);
  fs.write(buff, count);
  fs.flush();
  if (!fs.good()) {
    throw runtime_error("Fstream error: write");
  }
}

#endif
