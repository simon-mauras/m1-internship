#ifndef _INCLUDE_DATABASE
#define _INCLUDE_DATABASE

#include "cache.cpp"
#include <functional>

template <class Key, class T, class Compare = std::less<Key> >
class Database
{
  public:
    Database(Cache* c);
    virtual ~Database() {}
    virtual void insert(Key key, T value) = 0;
    virtual void remove(Key key) = 0;
    virtual T get(Key key) = 0;
  
  protected:
    Cache* cache;
};

template <class Key, class T, class Compare>
Database<Key, T, Compare>::Database(Cache* c)
{
  cache = c;
}

#endif
