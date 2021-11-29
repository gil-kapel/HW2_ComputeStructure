#include <vector>
using namespace std;

class block{
    int block_id;
    vector<int> block_data;
    bool dirty_bit;
public:
    block(const int* first_ptr);
    ~block();
    block(const block& block);
    block& operator=(const block& block);
    block& getBlockByID(int block_id);
    void updateBlock(const block& new_block);
    int getValue(int offset); 
};

class cache{
    vector<block> cache_data;
    int size;
    int assoc;
    bool WrAlloc;
    int cycle;
    int MemCyc;
    int missCount;
    int hitCount;
public:
    cache(int size, int assoc, bool WrAlloc, int cycle, int MemCyc);
    ~cache();
    int getOffest(uint32_t* ptr); /* How much bits to search after the beggining of the block */
    int getSet(uint32_t* ptr); /* which cell will the address (which floor) */ 
    int getTag(uint32_t* ptr); /* Msb - memory block id */
    bool isBlockInCache(uint32_t* ptr); //increase hit or miss count
    bool snoopRequest();

    double calculateMissRate();
    double calculateHitRate(){ return 1 - calculateMissRate(); }
    double averageAccessTime();
};

cache L1;
cache L2;


