#ifndef BLOCKCACHE_H
#define BLOCKCACHE_H
#include <assert.h>
#include <unordered_map>

enum replacementPolicy { FIFO, LRU };

template <typename T> class Block {

  std::string *_decompressedStr;
  T *_data;
  size_t _size;
  int _accessCount = 0;
  int _insertTime;
  bool _dataAllocated = false;

public:
  Block() { _decompressedStr = new std::string; };
  Block(T *);

  void setSize(int);
  size_t size() const;
  void access();
  void setInsertTime(int);
  int getInsertTime() const;
  void setData();
  T *data() const;
  std::string *decompressedStr() const;

  T &operator[](const int) const;
};

template <typename T> class BlockCache {
  // The actual cache
  std::unordered_map<int, Block<T>> _cache;

  typename std::unordered_map<int, Block<T>>::iterator _lastAccPtr;
  int _lastAccId = -1;

  // A counter to capture number of insertions
  // this is also propagated to each individual block
  // to serve as an insertion timestamp for the block
  int _insertions;
  int _deletions;

  // Cache size in BLOCKS
  int _size = 32;

  // Cache replacement policy
  replacementPolicy _rPolicy = FIFO;

public:
  BlockCache(){};

  BlockCache(int);

  BlockCache(int, replacementPolicy);

  void flushCache();

  int maxSize() const;

  void setMaxSize(int);

  int size() const;

  bool insert(int, Block<T> &);

  size_t remove(int);

  Block<T> &operator[](const int);

  typename std::unordered_map<int, Block<T>>::iterator find(int);

  typename std::unordered_map<int, Block<T>>::iterator getEnd() {
    return _cache.end();
  };

  T *access(int);

  T *access(typename std::unordered_map<int, Block<T>>::iterator);

  void printInfo() const;

  ~BlockCache();
};

#include "blockcache_impl.h"

#endif
