#ifndef CACHE_H_
#define CACHE_H_


#include <vector>
#include <math.h>
#include <list>
#include <stdint.h>

using namespace std;
#define INT_MAX 2147483647
int current_time = 0; //counter for block accessing

int getBlockIDByAddr(const uint32_t addr, const int block_size){
    return addr / block_size;
}

uint32_t getBlockFirstAddr(int block_id, int block_size){
    return block_id * block_size;
}

uint32_t getOffsetBits (uint32_t addr, int block_size){
    int num_of_bits = log2(block_size);
    int to_compare = pow(2, num_of_bits) - 1;
    return (addr & to_compare);
}

uint32_t getSetBits (uint32_t addr, int associativity, int block_size, int cache_size) {
    int offsetBits = log2(block_size);
    uint32_t set = addr >> offsetBits;
    int num_of_bits = log2(cache_size / (block_size * associativity));
    int to_compare = pow(2, num_of_bits) - 1;
    return (set & to_compare);
}

uint32_t getTagBits (uint32_t addr, int associativity, int block_size, int cache_size){
    int bits_to_remove = log2(block_size);
    bits_to_remove += log2(cache_size / (block_size * associativity));
    uint32_t tag = addr >> bits_to_remove;
    int num_of_bits = 32 - bits_to_remove;
    int to_compare = pow(2, num_of_bits) - 1;
    return (tag & to_compare);
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
    int getBlockID()const {return block_id; }
    void makeClean();
    void readBlock();
    void writeToBlock();
    bool isAddrInBlock(const uint32_t addr)const;
    uint32_t getFirstAddr()const { return first_addr; }
    bool compareBlockAddress(const uint32_t addr)const { return block_id == getBlockIDByAddr(addr, block_size); }
    bool isBlockDirty()const { return dirty_bit; }
    //int getValue(int offset); 
    int getLastAccess() const { return last_access; }
};

Block::Block():block_id(-1), first_addr(-1), block_size(0), dirty_bit(false), last_access(0){};

Block::Block(const uint32_t addr, const int block_size, bool is_dirty): block_size(block_size),
                                                                        last_access(current_time), dirty_bit(is_dirty){
    block_id = getBlockIDByAddr(addr, block_size);
    first_addr = getBlockFirstAddr(block_id, block_size);
    current_time++;
}

Block::Block(const Block& block): block_id(block.block_id), block_size(block.block_size), first_addr(block.first_addr),
                                    last_access(block.last_access), dirty_bit(block.dirty_bit){};

Block& Block::operator=(const Block& block){
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

class CacheEntry{
    vector<Block> line;
public:
    CacheEntry(int size){
        for(int i=0 ; i < size ; i++){
            line.push_back(Block());
        }
    }
    ~CacheEntry() = default;
    vector<Block>& getLine(){ return line; }
};

class Cache{
    vector<CacheEntry> cache_data;
    int cache_size;
    int num_of_lines;
    int assoc;
    double missCount;
    double hitCount;
public:
    int block_size;
    Cache(int cache_size, int block_size, int assoc): cache_size(pow(2, cache_size)), block_size(pow(2, block_size)), assoc(pow(2,assoc)){
        missCount = 0;
        hitCount = 0;
        for(int i = 0 ; i < this->assoc ; i++){
            cache_data.push_back(CacheEntry(this->cache_size / (this->block_size * this->assoc)));
        }
    }
    ~Cache() = default;
    bool isBlockInCache(const uint32_t addr); //increase hit or miss count
    void addBlock(const Block& block);
    void removeBlock(const Block& block);
    Block& getBlockFromAddr(const uint32_t addr);
    Block get_LRU_BlockFromSameLine(const uint32_t addr);
    vector<Block>& findLine(const uint32_t addr);
    void updateBlock(const Block& block);
    void updateValue(double* miss_rate) { *miss_rate = missCount / (missCount + hitCount) ;}
    double calculateMissRate() { return missCount / (missCount + hitCount); } /* Need to verify the equation */
    double calculateHitRate(){ return 1 - calculateMissRate(); }
    double averageAccessTime();
};

bool Cache::isBlockInCache(const uint32_t addr){
    int addr_set = getSetBits(addr, assoc, block_size, cache_size);
    int addr_tag = getTagBits(addr, assoc, block_size, cache_size);

    if(assoc == cache_size / block_size){
        for(auto &line: cache_data){
            if(addr_tag == getTagBits(line.getLine()[0].getFirstAddr(), assoc, block_size, cache_size)){
                hitCount++;
                return true;
            }
        }
        missCount++;
        return false;
    }
    else {
        for(auto &line: cache_data){
            for(auto &block: line.getLine()){
                if(addr_tag == getTagBits(line.getLine()[addr_set].getFirstAddr(), assoc, block_size, cache_size)){ 
                    hitCount++;
                    /* Another act? */
                    return true;
                } 
            }
        }
        missCount++;
        return false;
    }
    missCount++;
    return false;
}

void Cache::addBlock(const Block& block){
    int set = getSetBits(block.getFirstAddr(), assoc, block_size, cache_size);
    int tag = getTagBits(block.getFirstAddr(), assoc, block_size, cache_size);

    if(assoc == cache_size / block_size){
        int size = cache_size / block_size;
        for(int i= 0 ; i <  size ; i++){
            if(cache_data[i].getLine()[0].getBlockID() == -1){
                cache_data[i].getLine()[0] = block;
                return;
            }
            else if(tag == getTagBits(cache_data[i].getLine()[0].getFirstAddr(), assoc, block_size, cache_size)){
                cache_data[i].getLine()[0] = block;
                return;
            }
        }
    }
    
    else{
        for(auto &line: cache_data){
            if(line.getLine()[set].getBlockID() == -1){
                line.getLine()[set] = block;
                return;
            }
            else if(tag == getTagBits(line.getLine()[set].getFirstAddr(), assoc, block_size, cache_size)){
                line.getLine()[set] = block;
                return;
            }
        }
    }
}

void Cache::removeBlock(const Block& block){
    int set = getSetBits(block.getFirstAddr(), assoc, block_size, cache_size);
    int tag = getTagBits(block.getFirstAddr(), assoc, block_size, cache_size);
    
    if(assoc == cache_size / block_size){
        int size = cache_size / block_size;
        for(int i= 0 ; i < size ; i++){
            if(cache_data[i].getLine()[0].getBlockID() == -1) return;
            else if(tag == getTagBits(cache_data[i].getLine()[0].getFirstAddr(), assoc, block_size, cache_size)){
                cache_data[i].getLine()[0] = Block();
                return;
            }
        }
    }
    
    else{
        for(auto &line: cache_data){
            if(line.getLine()[set].getBlockID() == -1) return;
            else if(tag == getTagBits(line.getLine()[set].getFirstAddr(), assoc, block_size, cache_size)){
                line.getLine()[set] = Block();
                return;
            }
        }
    }
}

Block& Cache::getBlockFromAddr(const uint32_t addr){
    int set = getSetBits(addr, assoc, block_size, cache_size);
    int tag = getTagBits(addr, assoc, block_size, cache_size);
    
    if(assoc == cache_size / block_size){
        int size = cache_size / block_size;
        for(int i = 0 ; i < size ; i++){
            if(tag == getTagBits(cache_data[i].getLine()[0].getFirstAddr(), assoc, block_size, cache_size)){
                return cache_data[i].getLine()[0];
            }
        }
    }
    
    else{
        for(auto &line: cache_data){
            if(tag == getTagBits(line.getLine()[set].getFirstAddr(), assoc, block_size, cache_size)){
                return line.getLine()[set];
            }
        }
    }
    return cache_data[0].getLine()[0];
}

Block Cache::get_LRU_BlockFromSameLine(const uint32_t addr){
    int set = getSetBits(addr, assoc, block_size, cache_size);
    int tag = getTagBits(addr, assoc, block_size, cache_size);
    
    int glob_last_access = INT_MAX;
    Block lru_block = cache_data[0].getLine()[0];

    if(assoc == cache_size / block_size){
        int size = cache_size / block_size;
        for(int i = 0 ; i < size ; i++){
            if( cache_data[i].getLine()[0].getBlockID() == -1 ||                                               //return the first empty cell
                tag == getTagBits(cache_data[i].getLine()[0].getFirstAddr(), assoc, block_size, cache_size)){  // or the cell with the same tag
                return cache_data[i].getLine()[0];
            }
        }
    }
    
    else{
        for(auto &line: cache_data){
            if(line.getLine()[set].getBlockID() == -1){ //return the first empty cell
                return line.getLine()[set];
            }
            int cur_last_access = line.getLine()[set].getLastAccess(); // if the cell isn't empty, find the last recent used
            if(cur_last_access < glob_last_access){
                lru_block = line.getLine()[set];
                glob_last_access = cur_last_access;
            }
        }
    }
    return lru_block;
}

void Cache::updateBlock(const Block& block){
    int set = getSetBits(block.getFirstAddr(), assoc, block_size, cache_size);
    int tag = getTagBits(block.getFirstAddr(), assoc, block_size, cache_size);
    int size = cache_size / block_size;
    if(assoc == size){
        for(int i = 0 ; i < size ; i++){
            if(cache_data[i].getLine()[0].getBlockID() == -1) return;  //won't happen, checked in upper functions
            else if(tag == getTagBits(cache_data[i].getLine()[0].getFirstAddr(), assoc, block_size, cache_size)){
                cache_data[i].getLine()[0].writeToBlock();
                return;
            }
        }
    }
    
    else{
        for(auto &line: cache_data){
            if(line.getLine()[set].getBlockID() == -1) return;  //won't happen, checked in upper functions
            else if(tag == getTagBits(line.getLine()[set].getFirstAddr(), assoc, block_size, cache_size)){
                line.getLine()[set].writeToBlock();
                return;
            }
        }
    }
}
    
#endif // _CACHE_H
