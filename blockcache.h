#include <map>

enum replacementPolicy {
    FIFO,
    LRU
};

struct BlockId{
    int matId; 
    int blockId;
};

struct IDCompare{
    bool operator() (const BlockId& lhs, const BlockId& rhs){
        return ((lhs.matId < rhs.matId) || ((lhs.matId == rhs.matId) && (lhs.blockId < rhs.blockId)));
    }
};


template<typename T>
class Block{
    int _size;
    bool _dataAllocated = false;
    int _accessCount = 0;
    int _insertTime;
    T* _data;
    std::string* _decompressedStr;

    public:
    Block(){ _decompressedStr = new std::string; }; 
    void setSize(int);
    int getSize(); 
    void access();
    void setInsertTime(int);
    int getInsertTime();
    void setData();
    // Not safe? 
    T* getData();
    std::string* getDecompressedStr();

    T& operator [](int);

    ~Block();
};

template<typename T>
class BlockCache{
    // A counter to capture number of insertions
    // this is also propagated to each individual block
    // to serve as an insertion timestamp for the block   
    int _insertions;

    int _deletions;
    // Cache size in BLOCKS
    // TODO: change to KB ? 
    int _size = 32;

    // Cache replacement policy
    replacementPolicy _rPolicy = FIFO;

    // The actual cache
    //std::map<BlockId, Block<T>, IDCompare> _cache;
    std::map<int, Block<T>*> _cache;

    public:

    BlockCache(){};

    BlockCache(int);

    BlockCache(int, replacementPolicy);

    void flushCache(); 

    int getMaxSize();

    void setMaxSize(int);

    int getSize();

    //bool insert(BlockId, Block<T>);
    bool insert(int, Block<T>*);

    //size_t remove(BlockId);
    size_t remove(int);

    Block<T>& operator[](int);

    //typename std::map<BlockId, Block<T>, IDCompare>::iterator find(BlockId);
    typename std::map<int, Block<T>*>::iterator find(int);

    //typename std::map<BlockId, Block<T>, IDCompare>::iterator getEnd() { return _cache.end(); };
    typename std::map<int, Block<T>*>::iterator getEnd() { return _cache.end(); };

    // Not safe?
    //T*& access(BlockId);
    T* access(int);

    //T*& access(typename std::map<BlockId, Block<T>, IDCompare>::iterator);
    T* access(typename std::map<int, Block<T>*>::iterator);
   
    void printInfo();

    ~BlockCache();

};

#include "blockcache_impl.h"
