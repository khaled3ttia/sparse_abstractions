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
bool BlockCache<T>::insert(int bid, Block<T>* block){
    
    assert(_size > 0);
    
    if (_cache.size() < _size){
         _insertions++;
         block->setInsertTime(_insertions);
         _cache.insert(std::make_pair(bid, block));
         return true;
    }else {
        
        // TODO: implement other replacement policies here
                
        auto oldest = _cache.begin();

        /*
        auto it = std::next(oldest);


        while (it != _cache.end()){
            // This comparsion is a hotspot 
            if (it->second->getInsertTime() < oldest->second->getInsertTime()){
                oldest = it;
            }
            it++;
        }
        */
        BlockCache<T>::remove(oldest->first);
        BlockCache<T>::insert(bid, block);
    }
    return false;
}

template<typename T>
size_t BlockCache<T>::remove(int bid){
    _deletions++;
    delete _cache[bid];
    return _cache.erase(bid);
}

template<typename T>
Block<T>& BlockCache<T>::operator[](int bid){
    return *(_cache[bid]);
}

template<typename T>
T* BlockCache<T>::access(int bid){

    auto it = _cache.find(bid);

    if (it != _cache.end()){
        
        it->second.access();

        return it->second.getData();
    }

}

template<typename T>
T* BlockCache<T>::access(typename std::unordered_map<int, Block<T>*>::iterator it){

    it->second.access(); 
    return it->second.getData();

} 

template<typename T>
typename std::unordered_map<int, Block<T>*>::iterator BlockCache<T>::find(int bid){
    return _cache.find(bid);
}

template<typename T>
void BlockCache<T>::flushCache(){
    auto it = _cache.begin();
    while (it != _cache.end()){

        delete it->second;

        it++;
    }
    _cache.clear();
    _insertions = 0;
    _deletions = 0;
}

template<typename T>
void BlockCache<T>::printInfo(){
    std::cout << "===================BLOCKCACHE INFO====================" << std::endl;
    std::cout << "Cache Size: " << _cache.size() << " blocks" <<  std::endl;
    std::cout << "Total Insertions: " << _insertions << std::endl;
    std::cout << "Total Deletions: " << _deletions << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "Cache Contents:" << std::endl;
    std::cout << "==============" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    auto it = _cache.begin();
    std::cout << "| bid\t|\tBlock Address\t|" << std::endl; 
    std::cout << "--------------------------------" << std::endl;
    while (it != _cache.end()){
        std::cout <<"| " <<  it->first << "\t|\t" << it->second << "\t|" << std::endl;
        it++;
    }
    std::cout << "--------------------------------" << std::endl;



}

template<typename T>
BlockCache<T>::~BlockCache(){

    auto it = _cache.begin();
    while (it != _cache.end()){
       
        delete it->second;
        it++;

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
T* Block<T>::getData(){
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
void Block<T>::setData(){

    _data = (T*)_decompressedStr->c_str();
    _dataAllocated = true;
}

template<typename T>
std::string* Block<T>::getDecompressedStr(){
    return _decompressedStr;
}

template<typename T>
T& Block<T>::operator [](int idx){
    
    assert(_dataAllocated);

    return _data[idx];
}

template<typename T>
Block<T>::~Block(){
    delete _decompressedStr;
}
