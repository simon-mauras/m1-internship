#ifndef _INCLUDE_DATABASE_BST
#define _INCLUDE_DATABASE_BST

#include "database.cpp"
#include <functional>
#include <stdexcept>
#include <list>

using namespace std;

template <class Key, class T, class Compare = std::less<Key> >
class Database_bst: public Database<Key, T, Compare>
{
  public:
    Database_bst(Cache* c);
    void insert(Key key, T value);
    void remove(Key key);
    T get(Key key);
  
  protected:
    Compare compare;
    size_t find_node(Key key);
    size_t new_node(Key key, T value, size_t up);
    void update_up(size_t, size_t);
    void update_down(size_t, size_t, size_t);
    const size_t undefined = -1;
    class Node {
      public:
        size_t up, left, right;
        Key key;
        T value;
    };
};

template <class Key, class T, class Compare>
Database_bst<Key,T,Compare>::Database_bst(Cache* c): Database<Key,T,Compare>(c)
{
  this->compare = Compare();
}

template <class Key, class T, class Compare>
size_t Database_bst<Key,T,Compare>::find_node(Key key)
{
  size_t nb_nodes;
  this->cache->read((char*)&nb_nodes, 0, sizeof(size_t));
  
  if (nb_nodes == 0) {
    throw runtime_error("Key not found");
  }
  else
  {
    size_t pos = sizeof(size_t);
    while (pos != undefined)
    {
      Node node;
      this->cache->read((char*)&node, pos, sizeof(Node));
      if (this->compare(key, node.key))
        pos = node.left;
      else if (this->compare(node.key, key))
        pos = node.right;
      else
        return pos;
    }
    throw runtime_error("Key not found");
  }
}

template <class Key, class T, class Compare>
size_t Database_bst<Key,T,Compare>::new_node(Key key, T value, size_t up)
{
  size_t nb_nodes;
  this->cache->read((char*)&nb_nodes, 0, sizeof(size_t));
  
  Node node;
  node.key = key;
  node.value = value;
  node.up = up;
  node.left = node.right = undefined;
  size_t pos = sizeof(size_t) + nb_nodes * sizeof(Node);
  this->cache->write((char*)&node, pos, sizeof(Node));
  
  nb_nodes++;
  this->cache->write((char*)&nb_nodes, 0, sizeof(size_t));
  
  return pos;
}

template <class Key, class T, class Compare>
void Database_bst<Key,T,Compare>::insert(Key key, T value)
{
  size_t nb_nodes;
  this->cache->read((char*)&nb_nodes, 0, sizeof(size_t));
  
  if (nb_nodes == 0) {
    new_node(key, value, undefined);
  }
  else
  {
    size_t pos = sizeof(size_t);
    while (true)
    {
      Node node;
      this->cache->read((char*)&node, pos, sizeof(Node));
      if (this->compare(key, node.key)) {
        if (node.left == undefined) {
          node.left = new_node(key, value, pos);
          this->cache->write((char*)&node, pos, sizeof(Node));
          break;
        }
        pos = node.left;
      } else if (this->compare(node.key, key)) {
        if (node.right == undefined) {
          node.right = new_node(key, value, pos);
          this->cache->write((char*)&node, pos, sizeof(Node));
          break;
        }
        pos = node.right;
      } else {
        node.value = value;
        this->cache->write((char*)&node, pos, sizeof(Node));
        break;
      }
    }
  }
}

template <class Key, class T, class Compare>
void Database_bst<Key,T,Compare>::update_up(size_t pos, size_t up)
{
  Node node;
  this->cache->read((char*)&node, pos, sizeof(Node));
  node.up = up;
  this->cache->write((char*)&node, pos, sizeof(Node));
}

template <class Key, class T, class Compare>
void Database_bst<Key,T,Compare>::update_down(size_t pos, size_t x, size_t y)
{
  Node node;
  this->cache->read((char*)&node, pos, sizeof(Node));
  if (node.left == x) node.left = y;
  if (node.right == x) node.right = y;
  this->cache->write((char*)&node, pos, sizeof(Node));
}

template <class Key, class T, class Compare>
void Database_bst<Key,T,Compare>::remove(Key key)
{
  // Number of nodes
  size_t nb_nodes;
  this->cache->read((char*)&nb_nodes, 0, sizeof(size_t));
  
  // Find the node to delete (throw an exception if not found)
  Node node;
  size_t pos = find_node(key);
  this->cache->read((char*)&node, pos, sizeof(Node));
  
  // First, make sure that the node to delete has at most 1 children
  if (node.left != undefined && node.right != undefined)
  {
    Node node2;
    size_t pos2 = node.left;
    this->cache->read((char*)&node2, pos2, sizeof(Node));
    
    while (node2.right != undefined)
    {
      pos2 = node2.right;
      this->cache->read((char*)&node2, pos2, sizeof(Node));
    }
    
    node.key = node2.key;
    node.value = node2.value;
    this->cache->write((char*)&node, pos, sizeof(Node));
    
    pos = pos2;
    node = node2;
  }
  
  // Find the children of the node to delete
  size_t pos2 = undefined;
  if (node.left != undefined) pos2 = node.left;
  if (node.right != undefined) pos2 = node.right;
  
  // Disconnect the node
  if (node.up != undefined)
  {
    update_down(node.up, pos, pos2);
    if (pos2 != undefined)
      update_up(pos2, node.up);
  }
  else
  {
    this->cache->read((char*)&node, pos2, sizeof(Node));
    node.up = undefined;
    this->cache->write((char*)&node, pos, sizeof(Node));
    if (node.left != undefined) update_up(node.left, pos);
    if (node.right != undefined) update_up(node.right, pos);
    pos = pos2;
  }
  
  // Now the node is disconnected, we can delete it
  pos2 = sizeof(size_t) + (nb_nodes-1) * sizeof(Node);
  if (pos != pos2)
  {
    this->cache->read((char*)&node, pos2, sizeof(Node));
    this->cache->write((char*)&node, pos, sizeof(Node));
    if (node.up != undefined) update_down(node.up, pos2, pos);
    if (node.left != undefined) update_up(node.left, pos);
    if (node.right != undefined) update_up(node.right, pos);
  }
  
  // Decrease the number of nodes
  nb_nodes--;
  this->cache->write((char*)&nb_nodes, 0, sizeof(size_t));
}

template <class Key, class T, class Compare>
T Database_bst<Key,T,Compare>::get(Key key)
{
  Node node;
  size_t pos = find_node(key);
  this->cache->read((char*)&node, pos, sizeof(Node));
  return node.value;
}

#endif
