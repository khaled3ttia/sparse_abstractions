template <typename T> BlockCache<T>::BlockCache(int size) : _size(size) {}

template <typename T>
BlockCache<T>::BlockCache(int size, replacementPolicy rPolicy)
    : _size(size), _rPolicy(rPolicy) {}

template <typename T> int BlockCache<T>::maxSize() const { return _size; }

template <typename T> void BlockCache<T>::setMaxSize(int val) { _size = val; }

template <typename T> int BlockCache<T>::size() const { return _cache.size(); }

template <typename T> bool BlockCache<T>::insert(int bid, Block<T> &block) {

  assert(_size > 0);

  if (_cache.size() < _size) {
    _insertions++;
    block.setInsertTime(_insertions);
    _cache.emplace(bid, block);
    return true;
  } else {

    // TODO: implement other replacement policies here

    auto oldest = _cache.begin();
    BlockCache<T>::remove(oldest->first);
    BlockCache<T>::insert(bid, block);
  }
  return false;
}

template <typename T> size_t BlockCache<T>::remove(int bid) {
  _deletions++;
  delete _cache[bid].decompressedStr();
  return _cache.erase(bid);
}

template <typename T> Block<T> &BlockCache<T>::operator[](const int bid) {

  if (bid == _lastAccId) {

    return _lastAccPtr->second;
  }
  auto it = _cache.find(bid);
  _lastAccPtr = it;
  _lastAccId = bid;
  return it->second;
}

template <typename T> T *BlockCache<T>::access(int bid) {

  auto it = _cache.find(bid);

  if (it != _cache.end()) {

    it->second.access();

    return it->second.daata();
  }
}

template <typename T>
T *BlockCache<T>::access(
    typename std::unordered_map<int, Block<T>>::iterator it) {

  it->second.access();
  return it->second.data();
}

template <typename T>
typename std::unordered_map<int, Block<T>>::iterator
BlockCache<T>::find(int bid) {
  return _cache.find(bid);
}

template <typename T> void BlockCache<T>::flushCache() {
  auto it = _cache.begin();
  while (it != _cache.end()) {

    delete it->second.decompressedStr();
    it++;
  }
  _cache.clear();
  _insertions = 0;
  _deletions = 0;
}

#ifdef DEBUG
template <typename T> void BlockCache<T>::printInfo() {
  std::cout << "===================BLOCKCACHE INFO===================="
            << std::endl;
  std::cout << "Cache Size: " << _cache.size() << " blocks" << std::endl;
  std::cout << "Total Insertions: " << _insertions << std::endl;
  std::cout << "Total Deletions: " << _deletions << std::endl;
  std::cout << "------------------------------------------------------"
            << std::endl;
  std::cout << "Cache Contents:" << std::endl;
  std::cout << "==============" << std::endl;
  std::cout << "--------------------------------" << std::endl;
  auto it = _cache.begin();
  std::cout << "| bid\t|\tBlock Address\t|" << std::endl;
  std::cout << "--------------------------------" << std::endl;
  while (it != _cache.end()) {
    std::cout << "| " << it->first << "\t|\t" << it->second.data() << "\t|"
              << std::endl;
    it++;
  }
  std::cout << "--------------------------------" << std::endl;
}
#endif

template <typename T> BlockCache<T>::~BlockCache() {

  auto it = _cache.begin();
  while (it != _cache.end()) {

    delete it->second.data();
    it++;
  }
}

template <typename T> Block<T>::Block(T *src) {
  _data = src;
  _dataAllocated = true;
}

template <typename T> void Block<T>::setSize(int val) {
  assert(val > 0);
  _size = val;
}

template <typename T> size_t Block<T>::size() const { return _size; }

template <typename T> void Block<T>::access() { _accessCount++; }

template <typename T> T *Block<T>::data() const { return _data; }

template <typename T> void Block<T>::setInsertTime(int val) {
  assert(val > 0);
  _insertTime = val;
}

template <typename T> int Block<T>::getInsertTime() const {
  return _insertTime;
}

template <typename T> void Block<T>::setData() {

  _data = (T *)_decompressedStr->c_str();
  _dataAllocated = true;
}

template <typename T> std::string *Block<T>::decompressedStr() const {
  return _decompressedStr;
}

template <typename T> T &Block<T>::operator[](const int idx) const {

  return _data[idx];
}
