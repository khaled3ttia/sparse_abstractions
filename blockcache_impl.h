template<typename T>
BlockCache<T>::BlockCache(int size):_size(size){}

template<typename T>
BlockCache<T>::BlockCache(int size, replacementPolicy rPolicy): _size(size), _rPolicy(rPolicy){}

template<typename T>
int BlockCache<T>::getMaxSize(){
    return _size;
}

template<typename T>
void BlockCache<T>::setMaxSize(int val){
    _size = val;
}

template<typename T>
int BlockCache<T>::getSize(){
    return _cache.size();
}

template<typename T>
bool BlockCache<T>::insert(BlockId bid, Block<T> block){
    
    assert(_size > 0);
    

    // Record insetion order 
    _insertions++; 

    block.setInsertTime(_insertions); 

    if (_cache.size() < _size){
         _cache.insert(std::make_pair(bid, block));
         return true;
    }else {
        
        // TODO: implement other replacement policies here
                
        auto oldest = _cache.begin();
        auto it = std::next(oldest);

        while (it != _cache.end()){
            if (it->second.getInsertTime() < oldest->second.getInsertTime()){
                oldest = it;
            }
            it++;
        }
        _cache.insert(std::make_pair(bid, block));
        return true;
    }
    return false;
}


template<typename T>
size_t BlockCache<T>::remove(BlockId bid){
    return _cache.erase(bid);
}

template<typename T>
T*& BlockCache<T>::access(BlockId bid){

    auto it = _cache.find(bid);

    if (it != _cache.end()){
        
        it->second.access();

        return it->second.getData();
    }

}

template<typename T>
T*& BlockCache<T>::access(typename std::map<BlockId, Block<T>,IDCompare>::iterator it){

    it->second.access(); 
    return it->second.getData();

} 

template<typename T>
typename std::map<BlockId, Block<T>, IDCompare>::iterator BlockCache<T>::find(BlockId bid){
    return _cache.find(bid);
}

template<typename T>
Block<T>::Block(T*& src){
    
    if (!_dataAllocated){
        _data = new T[_size];
        _dataAllocated = true;
    }

    for (int i = 0; i < _size; i++){
        _data[i] = src[i];
    }
}

template<typename T>
Block<T>::Block(T*& src, int size){
    _size = size;
    if (!_dataAllocated){
        _data = new T[size];
        _dataAllocated = true;
    }
    for (int i = 0; i < size; i++){
        _data[i] = src[i];
    }
}

template<typename T>
void Block<T>::setSize(int val){
    assert(val > 0);
    _size = val;
}

template<typename T>
int Block<T>::getSize(){
    return _size;
}


template<typename T>
void Block<T>::access() {
    _accessCount++; 
}

template<typename T>
T*& Block<T>::getData(){
    return _data;
}

template<typename T>
void Block<T>::setInsertTime(int val){
    assert(val > 0);
    _insertTime = val;
}

template<typename T>
int Block<T>::getInsertTime(){
    return _insertTime;
}

template<typename T>
T& Block<T>::operator [](int idx){
    
    assert(idx < _size);

    assert(_dataAllocated);

    return _data[idx];
}
