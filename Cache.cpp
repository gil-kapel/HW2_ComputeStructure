#include <vector>
using namespace std;


class block{
    int index;
    int* first_ptr;
    vector<int> block_data;
    int offset;
public:
    block(const int* first_ptr);
    ~block();
};

class cache{
    vector<block> cache_data;
    int size;
    int assoc;
    bool WrAlloc;
    int cycle;
    int MemCyc;
public:
    cache();
    ~cache();
    
};

cache L1;
cache L2;


