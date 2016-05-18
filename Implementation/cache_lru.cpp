#ifndef _INCLUDE_CACHE_LRU
#define _INCLUDE_CACHE_LRU

#include "cache.cpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <list>

using namespace std;

class Cache_lru: public Cache
{
  public:
    Cache_lru(Storage* s);
    ~Cache_lru();
    void flush();
    void read(char* buff, size_t pos, size_t count);
    void write(char* buff, size_t pos, size_t count);
  
  protected:
    void page_unload();
    void page_load(size_t id);
    std::list<size_t> page_list;
    std::unordered_map<size_t, pair<char*, bool> > page_map;
};

Cache_lru::Cache_lru(Storage* s) : Cache(s)
{
  page_map.reserve(memorysize / blocksize);
}

void Cache_lru::page_unload()
{
  if (page_list.empty())
    throw runtime_error("Error");
  size_t id = page_list.back();
  size_t count = blocksize;
  if (page_map[id].second)
  {
    stats_output++;
    storage->write(page_map[id].first, id * count, count);
  }
  delete[] page_map[page_list.back()].first;
  page_map.erase(page_list.back());
  page_list.pop_back();
}

void Cache_lru::page_load(size_t id)
{
  list<size_t>::iterator it = find(page_list.begin(), page_list.end(), id);
  if (it == page_list.end())
  {
    stats_input++;
    page_map[id] = make_pair(new char[blocksize], false);
    storage->read(page_map[id].first, id * blocksize, blocksize);
    page_list.push_front(id);
    if (page_list.size() > memorysize / blocksize)
      page_unload();
  }
  else
  {
    page_list.erase(it);
    page_list.push_front(id);
  }
}

void Cache_lru::flush()
{
  while (!page_list.empty())
    page_unload();
}

void Cache_lru::read(char* buff, size_t pos, size_t count)
{
  for (size_t id=pos/blocksize; id<=(pos+count-1)/blocksize; id++)
  {
    page_load(id);
    size_t inf = max(blocksize*id, pos);
    size_t sup = min(blocksize*(id+1), pos+count);
    char* result = buff + inf - pos;
    char* begin = page_map[id].first + inf - blocksize*id;
    char* end = page_map[id].first + sup - blocksize*id;
    copy(begin, end, result);
  }
}

void Cache_lru::write(char* buff, size_t pos, size_t count)
{
  for (size_t id=pos/blocksize; id<=(pos+count-1)/blocksize; id++)
  {
    page_load(id);
    size_t inf = max(blocksize*id, pos);
    size_t sup = min(blocksize*(id+1), pos+count);
    char* result = page_map[id].first + inf - blocksize*id;
    char* begin = buff + inf - pos;
    char* end = buff + sup - pos;
    copy(begin, end, result);
    page_map[id].second = true;
  }
}

Cache_lru::~Cache_lru()
{
  flush();
}

#endif
