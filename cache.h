#ifndef CACHE_H_
#define CACHE_H_


#include <vector>
#include <list>

using namespace std;

class block{
    int block_id;
    vector<int> block_data;
    bool dirty_bit;
    int last_access;
public:
    block(const int* first_ptr);
    ~block();
    block(const block& block);
    block& operator=(const block& block);
    bool operator>(const block& block);
    bool operator<(const block& block);
    block& getBlockByID(int block_id);
    void updateBlock(const block& new_block); 
    int getValue(int offset); 
    int getPriority() { return last_access; }
    bool compareBlockAddress(int address); /* return if the address given is same as the block*/
    bool isBlockDirty() {return dirty_bit; }
    void setBlockDirty(bool dirty) { dirty_bit = dirty; }
    bool willMappedToSameLine(int address); // return if this block will be mapped to the same place as address

};

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