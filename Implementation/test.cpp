#include <iostream>
#include <fstream>
#include <cstdlib>
#include "storage_fstream.cpp"
#include "cache_lru.cpp"
#include "database_bst.cpp"
using namespace std;

typedef long long int ll;

const int dt = 100;

int main()
{
  Storage* s = new Storage_fstream("database.bin");
  Cache* c = new Cache_lru(s);
  Database<int,int>* d = new Database_bst<int,int>(c);
  
  ofstream stats;
  
  // MEMORY PARAMETERS
  c->set_memorysize(1<<20);
  c->set_blocksize(1<<10);
  
  // BUILD KEYS
  ll N = 100000;
  vector<int> keys;
  for (int i=0; i<N; i++)
    keys.push_back(i);
  
  // INSERT
  random_shuffle(keys.begin(), keys.end());
  stats.open("insert.csv");
  stats << "Nb,Input,Output" << endl;
  stats << "0,0,0" << endl;
  for (int i=0; i<N; i++)
  {
    d->insert(keys[i], keys[i]);
    if ((i+1) % dt == 0)
    {
      stats << i+1 << ",";
      stats << c->get_stats_input() << ",";
      stats << c->get_stats_output() << endl;
    }
  }
  stats.close();
  c->flush();
  c->print_stats();
  c->reset_stats();
  
  // FIND
  random_shuffle(keys.begin(), keys.end());
  ll checksum = N * (N-1) / 2;
  for (auto key : keys)
    checksum -= d->get(key);
  c->flush();
  c->print_stats();
  c->reset_stats();
  
  // DELETE
  random_shuffle(keys.begin(), keys.end());
  stats.open("delete.csv");
  stats << "Nb,Input,Output" << endl;
  for (int i=0; i<N; i++)
  {
    d->remove(keys[i]);
    if ((N-1-i) % dt == 0)
    {
      stats << N-1-i << ",";
      stats << c->get_stats_input() << ",";
      stats << c->get_stats_output() << endl;
    }
  }
  stats.close();
  c->flush();
  c->print_stats();
  c->reset_stats();
  
  cout << checksum << endl;
  
  delete d;
  delete c;
  delete s;
}
