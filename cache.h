#ifndef CACHE_H_
#define CACHE_H_


#include <vector>
#include <math.h>
using namespace std;
int current_time = 0; //counter for block accessing

class block{
    int block_id;
    uint32_t first_addr;
    int block_size;
    vector<int> block_data; // check if needed
    bool dirty_bit;
    int last_access;
public:
    block();
    block(const uint32_t addr, const int block_size, bool is_dirty = false);
    ~block() = default;
    //block(const block& block);
    //block& operator=(const block& block);
    bool operator>(const block& block);
    bool operator<(const block& block);
    bool operator==(int block_num);
    bool operator!=(int block_num);
    void makeDirty();
    void makeClean();
    void readBlock(uint32_t addr);
    void writeToBlock(uint32_t addr);
    bool isAddrInBlock(const uint32_t addr);
    //block& getBlockByID(int block_id);
    //void updateBlock(const block& new_block); 
    //int getValue(int offset); 
    //int getPriority() { return last_access; }
};

int getBlockIDByAddr (const uint32_t addr, const int block_size)
{
    uint32_t i = 0;
    int block_num = 0;
    while (i < addr)
    {
        i += block_size*8;
        block_num++;
    }
    return block_num;
}

uint32_t getBlockFirstAddr(int block_id, int block_size)
{
    return block_id*block_size;
}

block::block():block_id(-1), first_addr(0), block_size(0), dirty_bit(false), last_access(0){};

block::block(const uint32_t addr, const int block_size, bool is_dirty): block_size(block_size) ,
                                                                        last_access(current_time), dirty_bit(is_dirty)
{
    block_id = getBlockIDByAddr(addr, block_size);
    first_addr = getBlockFirstAddr(block_id, block_size);
    current_time++;
}

bool block::operator>(const block& block)
{
    return (this->block_id > block.block_id);
}

bool block::operator<(const block& block)
{
    return (this->block_id < block.block_id);
}

bool block::operator==(int block_num)
{
    return (this->block_id == block_num);
}

bool block::operator!=(int block_num)
{
    return (this->block_id != block_num);
}

void block::makeDirty()
{
    this->dirty_bit = true;
}

void block::makeClean()
{
    this->dirty_bit = false;
}

bool block::isAddrInBlock(const uint32_t addr)
{
    return ((addr < this->first_addr + 8*(this->block_size)) && addr >= this->first_addr);
}

void block::readBlock(uint32_t addr)
{
    if (isAddrInBlock(addr))
    {
        last_access = current_time;
        current_time++;
    }
}

void block::writeToBlock(uint32_t addr)
{
    if (isAddrInBlock(addr))
    {
        last_access = current_time;
        current_time++;
        makeDirty();
    }
}

uint32_t getOffsetBits (uint32_t addr, int block_size)
{
    int num_of_bits = log2(block_size*8);
    int to_compare = 1;
    int i = 0;
    while (i < num_of_bits)
    {
        to_compare= to_compare*2;
    }
    return (addr ^ to_compare);
}

uint32_t getSetBits (uint32_t addr, int associativity, int block_size)
{
    int offsetBits = log2(block_size*8);
    uint32_t set = addr >> offsetBits;
    int num_of_bits = log2(associativity*8);
    int to_compare = 1;
    int i = 0;
    while (i < num_of_bits)
    {
        to_compare= to_compare*2;
    }
    return (set ^ to_compare);
}

uint32_t getTagBits (uint32_t addr, int associativity, int block_size)
{
    int bits_to_remove = log2(block_size*8) + log2(associativity);
    uint32_t tag = addr >> bits_to_remove;
    int num_of_bits = 32 - log2(block_size*8) - log2(associativity);
    int to_compare = 1;
    int i = 0;
    while (i < num_of_bits)
    {
        to_compare= to_compare*2;
    }
    return (tag ^ to_compare);
}



////////////
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
    void replaceBlock(&&block.......); // When a block is removed, it mast be allocated to the lower levels
};

cache L1;
cache L2;


#endif