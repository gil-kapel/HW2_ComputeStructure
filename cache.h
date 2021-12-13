#ifndef CACHE_H_
#define CACHE_H_


#include <vector>
#include <math.h>
#include <list>

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



//////////// from gil's commit
//////////// from cache
// class cache{
//     vector<block> cache_data;
//     int size;
///////////////from block
//     block& getBlockByID(int block_id);
//     void updateBlock(const block& new_block); 
//     int getValue(int offset); 
//     int getPriority() { return last_access; }
//     bool compareBlockAddress(int address); /* return if the address given is same as the block*/
//     bool isBlockDirty() {return dirty_bit; }
//     void setBlockDirty(bool dirty) { dirty_bit = dirty; }
//     bool willMappedToSameLine(int address); // return if this block will be mapped to the same place as address

// };

class Cache{
    list<vector<block>> cache_data;
    int cache_size;
    int block_size;
    int assoc;
    bool WrAlloc;
    int cycle;
    int MemCyc;
    int missCount;
    int hitCount;
public:
    Cache(int cache_size, int block_size, int assoc, bool WrAlloc, int cycle, int MemCyc): cache_size(cache_size), block_size(block_size), assoc(assoc), WrAlloc(WrAlloc), cycle(cycle), MemCyc(MemCyc){
        missCount = 0;
        hitCount = 0;
        cache_data = list<vector<block>>(cache_size / (block_size * assoc));
        for(auto &line: cache_data){
            line = vector<block>(assoc);
        }
    }
    ~Cache() = default;
    bool isBlockInCache(int address); //increase hit or miss count
    void addBlock(int address);
    void removeBlock(int address);
    block& getBlockFromAddr(int address);
    block& get_LRU_BlockFromSameLine(int address);
    // void replaceBlock(&&block.......); // When a block is removed, it mast be allocated to the lower levels
    void updateValue(double* miss_rate) { *miss_rate = missCount / (missCount + hitCount) ;}

    bool snoopRequest();

    double calculateMissRate() { return missCount / (missCount + hitCount); } /* Need to verify the equation */
    double calculateHitRate(){ return 1 - calculateMissRate(); }
    double averageAccessTime();
};

bool Cache::isBlockInCache(int address){
    for(auto &line: cache_data){
        for(auto &block: line){
            if(block.compareBlockAddress(address)){
                hitCount++;
                /* Another act? */
                return true;
            } 
        }
    }
    return false;
}

block& Cache::getBlockFromAddr(int address){
    for(auto &line: cache_data){
        for(auto &block: line){
            if(block.compareBlockAddress(address)){
                return block;
            }
        }
    }
    return nullptr;
}

block& Cache::get_LRU_BlockFromSameLine(int address){
    for(auto &line: cache_data){
        for(auto &block: line){
            if(block.willMappedToSameLine(address)){
                return block;
            }
        }
    }
    return nullptr;
}

#endif // _CACHE_H
