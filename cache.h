#ifndef CACHE_H_
#define CACHE_H_


#include <vector>
#include <math.h>
#include <list>

using namespace std;
int current_time = 0; //counter for block accessing

int getBlockIDByAddr(const uint32_t addr, const int block_size){
    return addr / block_size;
}

uint32_t getBlockFirstAddr(int block_id, int block_size){
    return block_id * block_size;
}

class Block{
    int block_id;
    uint32_t first_addr;
    int block_size;
    // vector<int> block_data; // check if needed
    bool dirty_bit;
    int last_access;
public:
    Block();
    Block(const uint32_t addr, const int block_size, bool is_dirty = false);
    ~Block() = default;
    Block(const Block& block);
    Block& operator=(const Block& block);
    bool operator>(const Block& block);
    bool operator<(const Block& block);
    bool operator==(const Block& block);
    bool operator!=(const Block& block);
    void makeDirty();
    void makeClean();
    void readBlock();
    void writeToBlock();
    bool isAddrInBlock(const uint32_t addr)const;
    uint32_t getAddr()const { return first_addr; }
    bool compareBlockAddress(const uint32_t addr)const { return block_id == getBlockIDByAddr(addr, block_size); }
    bool isBlockDirty()const { return dirty_bit; }
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

Block::Block(const Block& block): block_id(block.block_id), block_size(block.block_size), first_addr(block.first_addr),
                                    last_access(block.last_access), dirty_bit(block.dirty_bit){};

Block& Block::operator=(const Block& block)
{
    if (this != &block)
    {
        this->first_addr = block.first_addr;
        this->last_access = block.last_access;
        this->dirty_bit = block.dirty_bit;
        this->block_id = block.block_id;
        this->block_size = block.block_size;
    }
    return *this;
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
    return (this->block_id != block.block_id);
}

void Block::makeDirty()
{
    this->dirty_bit = true;
}

void Block::makeClean()
{
    this->dirty_bit = false;
}

bool Block::isAddrInBlock(const uint32_t addr)const{
    return (addr < first_addr + block_size && addr >= first_addr);
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



uint32_t getOffsetBits (uint32_t addr, int block_size){
    int num_of_bits = log2(block_size*8);
    int to_compare = 1;
    int i = 0;
    while (i < num_of_bits){
        to_compare = to_compare*2;
    }
    return (addr ^ to_compare);
}

uint32_t getSetBits (uint32_t addr, int associativity, int block_size, int cache_size) {
    int offsetBits = log2(block_size*8);
    uint32_t set = addr >> offsetBits;
    int num_of_bits = log2(cache_size / (block_size * associativity));
    int to_compare = 1;
    int i = 0;
    while (i < num_of_bits){
        to_compare= to_compare*2;
    }
    return (set ^ to_compare);
}

uint32_t getTagBits (uint32_t addr, int associativity, int block_size, int cache_size)
{
    int bits_to_remove = log2(block_size*8) + log2(cache_size / (block_size * associativity));
    uint32_t tag = addr >> bits_to_remove;
    int num_of_bits = 32 - bits_to_remove;
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
    vector<Block>& getLine(){ return line; }
    int getLineId() { return line_id; }
};

class Cache{
    vector<CacheEntry> cache_data;
    int cache_size;
    int assoc;
    int missCount;
    int hitCount;
public:
    int block_size;
    Cache(int cache_size, int block_size, int assoc): cache_size(cache_size), block_size(block_size), assoc(assoc){
        missCount = 0;
        hitCount = 0;
        int line_in_use = 0;
        cache_data = vector<CacheEntry>(cache_size / (block_size * assoc));
        for(auto &line: cache_data){
            line = CacheEntry(assoc, line_in_use);
            line_in_use++;
        }
    }
    ~Cache() = default;
    bool isBlockInCache(const uint32_t addr); //increase hit or miss count
    void addBlock(const Block& block);
    void removeBlock(const Block& block);
    Block& getBlockFromAddr(const uint32_t addr);
    Block& get_LRU_BlockFromSameLine(const uint32_t addr);
    vector<Block>& findLine(const uint32_t addr);
    void updateBlock(const Block& block);
    void updateValue(double* miss_rate) { *miss_rate = missCount / (missCount + hitCount) ;}
    double calculateMissRate() { return missCount / (missCount + hitCount); } /* Need to verify the equation */
    double calculateHitRate(){ return 1 - calculateMissRate(); }
    double averageAccessTime();
    bool willMappedToSameLine(const uint32_t addr, const Block& block) {return (getSetBits(addr,assoc,block_size,cache_size) == getSetBits(block.getFirstAddr(),assoc,block_size,cache_size))};
};

bool Cache::isBlockInCache(const uint32_t addr){
    for(auto &line: cache_data){
        for(auto &block: line.getLine()){
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
    vector<Block> line = findLine(block.getAddr());
    const Block new_block = block;
    line.insert(new_block);
}

void Cache::removeBlock(const Block& block){
    vector<Block> line = findLine(block.getAddr());
    line.erase(block);
}

Block& Cache::getBlockFromAddr(const uint32_t addr){
    for(auto &line: cache_data){
        for(auto &block: line.getLine()){
            if(block.compareBlockAddress(addr)){
                return block;
            }
        }
    }
    return nullptr;
}

Block& Cache::get_LRU_BlockFromSameLine(const uint32_t addr){
    for(auto &line: cache_data){
        for(auto &block: line.getLine()){
            if(block.willMappedToSameLine(addr)){
                return block;
            }
        }
    }
    return nullptr;
}

vector<Block>& Cache::findLine(const uint32_t addr){
    for(auto &line: cache_data){
        for(auto &block: line.getLine()){
            if(block.compareBlockAddress(addr)){
                return line.getLine();
            }
        }
    }
    return nullptr;
}

void Cache::updateBlock(const Block& block){
    for(auto &line: cache_data){
        for(auto &old_block: line.getLine()){
            if(old_block.compareBlockAddress(block.getAddr())){
                // old_block = block;
                old_block.writeToBlock();
            }
        }
    }
}


#endif // _CACHE_H
