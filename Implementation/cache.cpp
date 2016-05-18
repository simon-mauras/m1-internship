#ifndef _INCLUDE_CACHE
#define _INCLUDE_CACHE

#include "storage.cpp"
#include <iostream>
#include <cstddef>

class Cache
{
  public:
    Cache(Storage* s);
    virtual ~Cache() {}
    void set_blocksize(size_t);
    void set_memorysize(size_t);
    virtual void flush() = 0;
    virtual void read(char* buff, size_t pos, size_t count) = 0;
    virtual void write(char* buff, size_t pos, size_t count) = 0;
    void print_stats();
    void reset_stats();
    int get_stats_input();
    int get_stats_output();
    size_t get_blocksize();
    size_t get_memorysize();
  
  protected:
    Storage* storage;
    int stats_input;
    int stats_output;
    size_t blocksize;
    size_t memorysize;
};

Cache::Cache(Storage* s)
{
  storage = s;
  reset_stats();
  blocksize = 1 << 10;  // 1KB
  memorysize = 1 << 20; // 1MB
}

void Cache::reset_stats()
{
  stats_input = 0;
  stats_output = 0;
}

void Cache::print_stats()
{
  std::cerr << "===== Stats =====" << std::endl;
  std::cerr << "Input: " << get_stats_input() << std::endl;
  std::cerr << "Output: " << get_stats_output() << std::endl;
}

int Cache::get_stats_input()
{
  return stats_input;
}

int Cache::get_stats_output()
{
  return stats_output;
}

void Cache::set_blocksize(size_t b)
{
  blocksize = b;
}

size_t Cache::get_blocksize()
{
  return blocksize;
}

void Cache::set_memorysize(size_t m)
{
  memorysize = m;
}

size_t Cache::get_memorysize()
{
  return memorysize;
}

#endif
