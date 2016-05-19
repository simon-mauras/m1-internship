#ifndef _INCLUDE_DATABASE_COLA
#define _INCLUDE_DATABASE_COLA

#include "database.cpp"
#include <functional>
#include <stdexcept>
#include <list>

using namespace std;

template <class Key, class T, class Compare = less<Key> >
class Database_cola: public Database<Key, T, Compare>
{
  public:
    Database_cola(Cache* c);
    void insert(Key key, T value);
    void remove(Key key);
    T get(Key key);
    void debug();
  
  protected:
    Compare compare;
    struct Element {
      Element() {}
      Element(bool t, Key k, T v) : type(t), key(k), value(v) {}
      bool type;
      Key key;
      T value;
    };
    void add(Element element);
    const size_t undefined = -1;
    const bool type_insert = true;
    const bool type_remove = false;
};

template <class Key, class T, class Cmp>
Database_cola<Key,T,Cmp>::Database_cola(Cache* c): Database<Key,T,Cmp>(c)
{
  this->compare = Cmp();
}

template <class Key, class T, class Compare>
void Database_cola<Key,T,Compare>::add(Element element)
{
  size_t nb_elems;
  this->cache->read((char*)&nb_elems, 0, sizeof(size_t));
  
  // Fill in the queues
  size_t id = 0;
  list<pair<Element, size_t> > queues;
  queues.push_back(make_pair(element, undefined));
  for (id = 0; (1lu<<id) & nb_elems; id++)
  {
    Element elem;
    size_t pos = sizeof(size_t) + ((1lu << id) - 1) * sizeof(Element);
    size_t next = (id == 0) ? undefined : pos + sizeof(Element);
    this->cache->read((char*)&elem, pos, sizeof(Element));
    queues.push_back(make_pair(elem, next));
  }
  
  // Merge
  size_t nb_discards = 0;
  size_t begin = sizeof(size_t) + ((1lu << id) - 1) * sizeof(Element);
  size_t end = begin;
  while (!queues.empty())
  {
    // Find the next key to insert
    Key best = queues.front().first.key;
    for (const auto &x : queues)
      if (compare(x.first.key, best))
        best = x.first.key;
    
    // Insert...
    bool inserted = false;
    for (auto it=queues.begin(); it!=queues.end();)
    {
      if (compare(it->first.key, best)) { it++; continue; }
      if (compare(best, it->first.key)) { it++; continue; }
      if (!inserted) {
        inserted = true;
        this->cache->write((char*)&it->first, end, sizeof(Element));
        end += sizeof(Element);
      } else {
        nb_discards++;
      }
      if (it->second == undefined) {
        it = queues.erase(it);
      } else {
        this->cache->read((char*)&it->first, it->second, sizeof(Element));
        size_t i = 2 + (it->second - sizeof(size_t)) / sizeof(Element);
        if ((i & -i) == i) // End of queue <=> i is a power of 2
          it->second = undefined;
        else
          it->second += sizeof(Element);
        it++;
      }
    }
  }
  
  // Move elements because of discards
  if (nb_discards != 0)
  {
    end = begin;
    size_t nb_moves = (1lu << id) - nb_discards;
    for (size_t i=0; (1lu << i) <= nb_moves; i++)
    {
      size_t pos = sizeof(size_t) + ((1lu << i) - 1) * sizeof(Element);
      for (size_t j=0; j<((1lu << i) & nb_moves); j++)
      {
        Element elem;
        this->cache->read((char*)&elem, end, sizeof(Element));
        this->cache->write((char*)&elem, pos, sizeof(Element));
        end += sizeof(Element);
        pos += sizeof(Element);
      }
    }
  }
  
  // Update number of elements
  nb_elems = nb_elems + 1 - nb_discards;
  this->cache->write((char*)&nb_elems, 0, sizeof(size_t));
}

template <class Key, class T, class Compare>
void Database_cola<Key,T,Compare>::debug()
{
  cout << "============" << endl;
  size_t nb_elems;
  this->cache->read((char*)&nb_elems, 0, sizeof(size_t));
  cout << nb_elems << endl;
  for (size_t i=0; (1lu << i) <= nb_elems; i++)
  {
    size_t pos = sizeof(size_t) + ((1lu << i) - 1) * sizeof(Element);
    for (size_t j=0; j<((1lu << i) & nb_elems); j++)
    {
      Element elem;
      this->cache->read((char*)&elem, pos, sizeof(Element));
      pos += sizeof(Element);
      cout << elem.key << "(" << elem.type << ") ";
    }
  }
  cout << endl;
}

template <class Key, class T, class Compare>
void Database_cola<Key,T,Compare>::insert(Key key, T value)
{
  add(Element(type_insert, key, value));
}

template <class Key, class T, class Compare>
void Database_cola<Key,T,Compare>::remove(Key key)
{
  add(Element(type_remove, key, T()));
}

template <class Key, class T, class Compare>
T Database_cola<Key,T,Compare>::get(Key key)
{
  size_t nb_elems;
  this->cache->read((char*)&nb_elems, 0, sizeof(size_t));
  for (size_t i=0; (1lu << i) <= nb_elems; i++)
  {
    if ((1lu << i) & nb_elems)
    {
      size_t begin = (1lu << i) - 1;
      size_t end = (1lu << (i+1)) - 1;
      while (begin < end)
      {
        Element elem;
        size_t med = (begin + end) / 2;
        size_t pos = sizeof(size_t) + med * sizeof(Element);
        this->cache->read((char*)&elem, pos, sizeof(Element));
        if (compare(key, elem.key))
          end = med;
        else if (compare(elem.key, key))
          begin = med + 1;
        else if (elem.type == type_insert)
          return elem.value;
        else
          throw runtime_error("Key not found");
      }
    }
  }
  throw runtime_error("Key not found");
}

#endif
