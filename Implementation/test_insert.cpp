#include <iostream>
#include <fstream>
#include <cstdlib>
#include "storage_fstream.cpp"
#include "cache_lru.cpp"
#include "database_bst.cpp"
#include "database_cola.cpp"
using namespace std;

typedef long long int ll;

int X = -1;
int B = 1 << 5;
const int T = 1;

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    cout << "Usage: " << argv[0] << " blocksize type" << endl;
    return 1;
  }
  else
  {
    B = atoi(argv[1]);
    if (string(argv[2]) == "bst") X = 1;
    if (string(argv[2]) == "cola") X = 2;
    if (X == -1 || B <= 0 || B > (1 << 20))
    {
      cout << X << endl;
      cout << "Error" << endl;
      return 1;
    }
  }
  
  Storage* s = new Storage_fstream("database.bin");
  Cache* c = new Cache_lru(s);
  Database<int,int>* d = nullptr;
  
  if (X == 1) d = new Database_bst<int,int>(c);
  if (X == 2) d = new Database_cola<int,int>(c);
  
  ofstream stats;
  
  // MEMORY PARAMETERS
  c->set_memorysize((1<<8) * B);
  c->set_blocksize(B);
  
  // BUILD KEYS
  ll N = 10000;
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
    //d->debug();
    if ((i+1) % T == 0)
    {
      stats << i+1 << ",";
      stats << c->get_stats_input() << ",";
      stats << c->get_stats_output() << endl;
    }
  }
  stats.close();
  c->flush();
  c->print_stats();
  
  delete d;
  delete c;
  delete s;
}
