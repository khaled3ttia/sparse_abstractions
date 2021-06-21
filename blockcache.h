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
    T* _data;
    bool _dataAllocated = false;
    int _accessCount = 0;
    int _insertTime;

    public:
    Block(){}; 
    Block(T*&);
    Block(T*&, int);
    void setSize(int);
    int getSize(); 
    void access();
    void setInsertTime(int);
    int getInsertTime();

    // Not safe? 
    T*& getData();

    T& operator [](int);
};

template<typename T>
class BlockCache{
    // A counter to capture number of insertions
    // this is also propagated to each individual block
    // to serve as an insertion timestamp for the block   
    int _insertions;

    // Cache size in BLOCKS
    // TODO: change to KB ? 
    int _size = 32;

    // Cache replacement policy
    replacementPolicy _rPolicy = FIFO;

    // The actual cache
    std::map<BlockId, Block<T>, IDCompare> _cache;

    public:

    BlockCache(){};

    BlockCache(int);

    BlockCache(int, replacementPolicy);

    int getMaxSize();

    void setMaxSize(int);

    int getSize();

    bool insert(BlockId, Block<T>);

    size_t remove(BlockId);

    typename std::map<BlockId, Block<T>, IDCompare>::iterator find(BlockId);

    typename std::map<BlockId, Block<T>, IDCompare>::iterator getEnd() { return _cache.end(); };

    // Not safe?
    T*& access(BlockId);
    T*& access(typename std::map<BlockId, Block<T>, IDCompare>::iterator);
};

#include "blockcache_impl.h"
