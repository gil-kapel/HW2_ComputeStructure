#ifndef CACHE_H_
#define CACHE_H_


#include <vector>
#include <math.h>
#include <list>

using namespace std;
int current_time = 0; //counter for block accessing

int getBlockIDByAddr (const uint32_t addr, const int block_size){
    uint32_t i = 0;
    int block_num = 0;
    while (i < addr)
    {
        i += block_size*8;
        block_num++;
    }
    return block_num;
}

uint32_t getBlockFirstAddr(int block_id, int block_size){
    return block_id*block_size;
}

class Block{
    int block_id;
    uint32_t first_addr;
    int block_size;
    vector<int> block_data; // check if needed
    bool dirty_bit;
    int last_access;
public:
    Block();
    Block(const uint32_t addr, const int block_size, bool is_dirty = false);
    ~Block() = default;
    //Block(const Block& block);
    //Block& operator=(const Block& block);
    bool operator>(const Block& block);
    bool operator<(const Block& block);
    bool operator==(const Block& block);
    bool operator!=(const Block& block);
    void makeDirty();
    void makeClean();
    void readBlock();
    void writeToBlock();
    bool isAddrInBlock(const uint32_t addr);
    uint32_t getAddr(){ return first_addr; }
    bool compareBlockAddress(const uint32_t addr){ return (this->block_id == getBlockIDByAddr(addr)); }
    // bool willMappedToSameLine(const uint32_t addr);
    bool isBlockDirty(){ return dirty_bit; }
    //Block& getBlockByID(int block_id);
    //int getValue(int offset); 
    //int getPriority() { return last_access; }
};


Block::Block():block_id(-1), first_addr(0), block_size(0), dirty_bit(false), last_access(0){};

Block::Block(const uint32_t addr, const int block_size, bool is_dirty): block_size(block_size),
                                                                        last_access(current_time), dirty_bit(is_dirty)
{
    block_id = getBlockIDByAddr(addr, block_size);
    first_addr = getBlockFirstAddr(block_id, block_size);
    current_time++;
}

bool Block::operator>(const Block& block){
    return (this->block_id > block.block_id);
}

bool Block::operator<(const Block& block){
    return (this->block_id < block.block_id);
}

bool Block::operator==(const Block& block){
    return (this->block_id == block.block_id);
}

bool Block::operator!=(const Block& block){
    return (! *this == block );
}

void Block::makeDirty()
{
    this->dirty_bit = true;
}

void Block::makeClean()
{
    this->dirty_bit = false;
}

bool Block::isAddrInBlock(const uint32_t addr)
{
    return ((addr < this->first_addr + 8*(this->block_size)) && addr >= this->first_addr);
}

void Block::readBlock(){
    last_access = current_time;
    current_time++;
}

void Block::writeToBlock(){
    last_access = current_time;
    current_time++;
    makeDirty();
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
class CacheEntry{
    vector<Block> line;
    int line_id; 
public:
    CacheEntry(int size, int line_id): line_id(line_id){
        line = vector<Block>(size);
    }
    ~CacheEntry() = default;
    vector<Block> getLine(){ return line; }
    int getLineId() { return line_id; }
};



class Cache{
    vector<CacheEntry> cache_data;
    int cache_size;
    int assoc;
    int missCount;
    int hitCount;
    int line_in_use;
public:
    int block_size;
    Cache(int cache_size, int block_size, int assoc): cache_size(cache_size), block_size(block_size), assoc(assoc){
        missCount = 0;
        hitCount = 0;
        line_in_use = 0;
        cache_data = vector<CacheEntry>(cache_size / (block_size * assoc));
        for(auto &line: cache_data){
            line = CacheEntry(assoc, line_in_use);
        }
    }
    ~Cache() = default;
    bool isBlockInCache(const uint32_t addr); //increase hit or miss count
    void addBlock(const Block& block);
    void removeBlock(const Block& block);
    Block& getBlockFromAddr(const uint32_t addr);
    Block& get_LRU_BlockFromSameLine(const uint32_t addr);
    vector<Block> findLine(const uint32_t addr);
    void updateBlock(const Block& block);
    void updateValue(double* miss_rate) { *miss_rate = missCount / (missCount + hitCount) ;}
    double calculateMissRate() { return missCount / (missCount + hitCount); } /* Need to verify the equation */
    double calculateHitRate(){ return 1 - calculateMissRate(); }
    double averageAccessTime();
};

bool Cache::isBlockInCache(const uint32_t addr){
    for(auto &line: cache_data){
        for(auto &block: line){
            if(block.compareBlockAddress(addr)){
                hitCount++;
                /* Another act? */
                return true;
            } 
        }
    }
    return false;
}

void Cache::addBlock(const Block& block){
    Block new_block = block;
    vector<Block> line = findLine(block.getAddr());
    line.insert(new_block);
}

void Cache::removeBlock(const Block& block){
    vector<Block> line = findLine(block.getAddr());
    line.erase(block);
}

Block& Cache::getBlockFromAddr(const uint32_t addr){
    for(auto &line: cache_data){
        for(auto &block: line){
            if(block.compareBlockAddress(addr)){
                return block;
            }
        }
    }
    return nullptr;
}

Block& Cache::get_LRU_BlockFromSameLine(const uint32_t addr){
    for(auto &line: cache_data){
        for(auto &block: line){
            if(block.willMappedToSameLine(addr)){
                return block;
            }
        }
    }
    return nullptr;
}

vector<Block> Cache::findLine(const uint32_t addr){
    for(auto &line: cache_data){
        for(auto &block: line){
            if(block.compareBlockAddress(addr)){
                return line;
            }
        }
    }
    return nullptr;
}

void Cache::updateBlock(const Block& block){
    for(auto &line: cache_data){
        for(auto &old_block: line){
            if(old_block.compareBlockAddress(block.getAddr())){
                // old_block = block;
                old_block.writeToBlock();
            }
        }
    }
}


#endif // _CACHE_H
