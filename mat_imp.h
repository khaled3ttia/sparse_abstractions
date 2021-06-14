
// Dummy default constructor, if no parameters passed 
// create a 10-element array
template <typename T>
mat<T>::mat() {
    
    _ddata = new T[10];
    _denseAllocated = true;
    _mtFormat = DENSE;
    for (int i = 0 ; i < 10 ; i++){
        _ddata[i] = i; 
    }
}


// Constructor that uses only number of rows, number of columns and matrix format
template <typename T>
mat<T>::mat(int nrows, int ncols, format mtFormat): _nrows(nrows), _ncols(ncols), _mtFormat(mtFormat){

    
    int nElems = nrows * ncols; 

    if (_mtFormat == DENSE) {

        _ddata = new T[nElems];
        
        for (int i = 0 ; i < nElems ; i++){
            _ddata[i] = i;

        }
        
        _denseAllocated = true;

    } //TODO (if _mtFormat == COO)

}

// Constructor for dense 
template <typename T> 
mat<T>::mat(int nrows, int ncols, int nnz, T *& data): _nrows(nrows), _ncols(ncols), _nnz(nnz){

    _mtFormat = DENSE; 

    _ddata = new T[_nrows*_ncols];

    _ddata = data; 

    _denseAllocated = true;


}

/*
// Constructor for COO 
template <typename T> 
mat<T>::mat(int nrows, int ncols, int nnz, T *& coovals, int *& cooRowIdx, int *& cooColIdx): _nrows(nrows), _ncols(ncols), _nnz(nnz){

    _mtFormat = COO; 

    _coovals = new T[_nnz];

    _coovals = coovals;

    _cooRowIdx = new int[_nnz];

    _cooRowIdx = cooRowIdx; 

    _cooColIdx = new int[_nnz];

    _cooColIdx = cooColIdx;

    _cooAllocated = true;


}
*/


// Constructor that loads the matrix from a mtx file, using the specified format
template <typename T>
mat<T>::mat(std::string matFile, int blockSizeRows, format mtFormat): _blockSizeRows(blockSizeRows), _mtFormat(mtFormat){
    
    assert(_nrows % _blockSizeRows == 0);


    int currentLine = 0; 
    int currentIndex = 0; 

    std::string line; 
    std::ifstream mtx_file (matFile);
    
  
    if (!mtx_file){
        // file does not exist
        return; 

    }
    

    if (mtx_file.is_open()){
        
        while (std::getline(mtx_file, line)) {
            
            std::istringstream iss(line);

            if (currentLine == 0) {
                currentLine++;
                continue;

            }else if (currentLine == 1) { 
                
                if (!(iss >> _nrows >> _ncols >> _nnz)) {return;}

                
                if (_mtFormat == COO) {

                    
                   if (_cooAllocated) {

                        delete [] _coovals; 
                        delete [] _cooRowIdx;
                        delete [] _cooColIdx; 

                    }

                    _coovals = new T[_nnz];
                    _cooRowIdx = new int[_nnz];
                    _cooColIdx = new int[_nnz];
                    _cooAllocated = true; 
                   


                }else if (_mtFormat == DENSE) {

                    if (_denseAllocated){
                        
                        delete [] _ddata;

                    }  
                    
                    int numElems = _ncols * _nrows; 
                    _ddata = new T[numElems];
                    for (int i = 0 ; i < numElems; i++){
                        _ddata[i] = 0;
                    }
                    _denseAllocated = true; 
                }

                currentLine++; 


            }else {
                int rowIdx, colIdx; 
                T val;

                if (!(iss >> rowIdx >> colIdx >> val)) {
                    this->~mat(); 
                    return;
                }  

                if (currentIndex < _nnz) {

                    if (_mtFormat == COO) {

                        _cooColIdx[currentIndex] = colIdx - 1;
                        _cooRowIdx[currentIndex] = rowIdx - 1;
                        _coovals[currentIndex] = val; 

                    }else if (_mtFormat == DENSE) {
                        _ddata[(rowIdx-1) * _ncols + (colIdx-1)] = val; 

                    }
                    currentIndex++;  

                }
                currentLine++; 
            }


        }
    }

    
    if (_mtFormat == DENSE){
        
        compressByRow(true);
        
    }

}


template <typename T>
mat<T>::mat(const mat<T> & rhs) : _ncols(rhs._ncols), _nrows(rhs._nrows), _nnz(rhs._nnz), _mtFormat(rhs._mtFormat){

    if (_mtFormat == DENSE) {
        size_t numElems = _ncols * _nrows;

        if (!_denseAllocated) {

            _ddata = new T[numElems];
            _denseAllocated = true;
            
            for (int i = 0 ; i < numElems; i++){

                _ddata[i] = rhs._ddata[i];
            }
        }
    } // TODO if (_mtFormat == COO)

}

template <typename T>
T& mat<T>::getCompressedElement(int rowIdx, int colIdx){

    // TODO: this should return a reference to the element
    // in the uncompressed block in cache

    int blockId = floor(rowIdx / _blockSizeRows);
    int rowWithinBlock = rowIdx % _blockSizeRows; 

    std::string uncompressedBlock;

    snappy::Uncompress(_compressedData[blockId].data(), _compressedData[blockId].size(), &uncompressedBlock);

    T* block = (T*)uncompressedBlock.c_str();
   
    return block[rowWithinBlock * _ncols + colIdx]; 


}

// Overloading the () operator
template <typename T>
T& mat<T>::operator () (int rowIdx, int colIdx) {

    assert(rowIdx < _nrows && colIdx < _ncols);

    if (_mtFormat == COO) {
        cooToDense();
    }
    
    if (_isCompressed){
        return getCompressedElement(rowIdx, colIdx);
     
        
    }

    return _ddata[rowIdx * _ncols + colIdx];

}

template <typename T>
T& mat<T>::operator [] (int flatIdx){

    assert(flatIdx < _nrows * _ncols);
    
    if (_mtFormat == COO){

        cooToDense();
    }

    return _ddata[flatIdx];

}

template <typename T>
mat<T> mat<T>::operator + (const mat<T> & rhs){

    if (!(_ncols == rhs._ncols && _nrows == rhs._nrows)){
            
        std::cerr << "Cannot add incompatible matrices" << std::endl;
        
        // TODO fix this
        return *this;

    }

    if (_mtFormat == DENSE && rhs._mtFormat == DENSE){

        int nnz = 0; 
        int numElems = _ncols * _nrows;

        T* newdata = new T[numElems];

        for (int i = 0 ; i < numElems ; i++){
            
            newdata[i] = _ddata[i] + rhs._ddata[i];

            if (newdata[i] != 0){
                
                nnz++; 
                
            }
            
        }

        return mat<T>(_nrows, _ncols, nnz, newdata);

    } // TODO: other combinations of formats: COO, DENSE


}

template <typename T>
mat<T> mat<T>::operator - (const mat<T> & rhs){

    if (!(_ncols == rhs._ncols && _nrows == rhs._nrows)){
            
        std::cerr << "Cannot add incompatible matrices" << std::endl;
        
        // TODO fix this
        return *this;

    }

    if (_mtFormat == DENSE && rhs._mtFormat == DENSE){

        int nnz = 0; 
        int numElems = _ncols * _nrows;

        T* newdata = new T[numElems];

        for (int i = 0 ; i < numElems ; i++){
            
            newdata[i] = _ddata[i] - rhs._ddata[i];

            if (newdata[i] != 0){
                
                nnz++; 
                
            }
            
        }

        return mat<T>(_nrows, _ncols, nnz, newdata);

    } // TODO: other combination of formats: COO, DENSE


}



template <typename T>
mat<T> mat<T>::operator * (const T rhs){

    if (_mtFormat == DENSE) {

        int numElems = _ncols * _nrows; 

        int nnz = 0;

        T* newdata = new T[numElems];

        for (int i = 0 ; i < numElems; i++){

            newdata[i] = _ddata[i] * rhs;

            if (newdata[i] != 0) {
                nnz++; 
            }

        }

        return mat<T>(_nrows, _ncols, nnz, newdata);

    }else if (_mtFormat == COO) {


        if (rhs == 0){

            std::cerr << "Multiplying a COO matrix by 0!" << std::endl;

            return *this;
        }

        T* newcoovals = new T[_nnz]; 
        
        for (int i = 0 ; i < _nnz ; i++){
            newcoovals[i] = _coovals[i] * rhs; 
            
        }

        return mat<T>(_nrows, _ncols, _nnz, newcoovals, _cooRowIdx, _cooColIdx); 


    }


}



// Overloading the = (copy operator)
template <typename T>
mat<T> & mat<T>::operator = (const mat<T> & rhs) {

    if (this != &rhs){

        _ncols = rhs._ncols;
        _nrows = rhs._nrows;
        _nnz = rhs._nnz;
        _mtFormat = rhs._mtFormat; 

        
        if (_mtFormat == DENSE) {
            int numElems = _ncols * _nrows;
            if (!_denseAllocated) { 
                
                _ddata = new T[numElems]; 
                _denseAllocated = true;
                for (int i = 0 ; i < numElems; i++){
                
                    _ddata[i] = rhs._ddata[i];

                }


            }else {
                delete [] _ddata; 
                T * _newddata = new T[numElems];
                
                for (int i = 0 ; i < numElems; i++){
                    _newddata[i] = rhs._ddata[i];

                }

                _ddata = _newddata;

            }
            
        }//TODO (if _mtFormat == COO) 

    }
    return *this;

}

template <typename T>
mat<T>::~mat<T>(){
    
    if (_denseAllocated){

        delete [] _ddata;
    }else if (_cooAllocated) {

        delete [] _coovals;
        delete [] _cooColIdx;
        delete [] _cooRowIdx; 
    }

}


template <typename T>
void mat<T>::cooToDense(){

    if (!_cooAllocated) {
        
        std::cerr << "Matrix not in COO format!" << std::endl;
        return; 

    }

    int numElems = _ncols * _nrows;
    if (!_denseAllocated){
        
        _ddata = new T[numElems];

        _denseAllocated = true; 
    }

    for (int i = 0 ; i < _nnz ; i++) {

        int row = _cooRowIdx[i];
        int col = _cooColIdx[i];

        _ddata[row * _ncols + col] = _coovals[i];

    }

    _mtFormat = DENSE;

    delete[] _coovals;
    delete[] _cooRowIdx;
    delete[] _cooColIdx;

    _cooAllocated = false;

}


template <typename T>
void mat<T>::denseToCoo() {

    if (!_denseAllocated) {

        std::cerr << "Matrix not in DENSE format!" << std::endl;
        return;
    }

    int numElems = _ncols * _nrows;


    
    // (if the matrix is not loaded from a .mtx file,
    // the value of _nnz will be 0)
    if (_nnz == 0){

        for (int i = 0 ; i < _nrows; i++){
            for (int j = 0 ; j < _ncols ; j++){

                if (this->operator()(i,j) != 0){
    
                    _nnz++;
                }

            }

        }

    }

    if (!_cooAllocated) {

        _coovals = new T[_nnz];
        _cooRowIdx = new int[_nnz];
        _cooColIdx = new int[_nnz];
        int currentIndex = 0;
        for (int i = 0 ; i < _nrows; i++){


            for (int j = 0; j < _ncols; j++){
                
                if (currentIndex >= _nnz){

                    break;
                }

                if (this->operator()(i,j) != 0 && currentIndex < _nnz){

                    _coovals[currentIndex] = this->operator()(i,j);
                    _cooRowIdx[currentIndex] = i;
                    _cooColIdx[currentIndex] = j;

                    currentIndex++; 
                }

            }


            if (currentIndex >= _nnz){
                break;
            }

        }
        
        _cooAllocated = true; 
        _mtFormat = COO; 


        delete [] _ddata;
        _denseAllocated = false; 
    }

}

template <typename T>
void mat<T>::compressByRow(bool removeOriginal) {


    if (_mtFormat == DENSE){
        
        if (!_denseAllocated){
            
            std::cerr << "Data for matrix not allocated yet!" << std::endl;
            return; 
        }
        
        // TODO (what if blockSize does not divide _nrows)

        int nBlocks = _nrows / _blockSizeRows;

        _compressedData = new std::string[nBlocks];

        size_t compLength[nBlocks];


        int blockOffset = 0; 

        T* block = new T[_ncols*_blockSizeRows];

        for (int i = 0 ; i < nBlocks; i++){


            block = _ddata + blockOffset;

            char * uncompressedBlock = (char*)block; 

           
            compLength[i] = snappy::Compress(uncompressedBlock, _ncols * sizeof(T) * _blockSizeRows, &_compressedData[i]);

            blockOffset += _ncols * _blockSizeRows;
            
        }


        _isCompressed = true;

        if (removeOriginal){
            
            delete [] _ddata;
            _denseAllocated = false;      

        }
    }


}

template <typename T>
void mat<T>::decompressByRow( bool removeCompressed) {
    
    // TODO: delete [] unused

    if (_mtFormat == DENSE) {

        if (!_isCompressed){
    
            std::cerr << "Matrix is not compressed!" << std::endl;
            return;
        }
        
        int nBlocks = _nrows / _blockSizeRows;

        size_t totalUncompressedBytes = 0; 

        std::string * uncompressedData = new std::string[nBlocks];
        
        size_t blocksSizes[nBlocks];

        for (int i = 0 ; i < nBlocks; i++){
            size_t partialSize;
            //snappy::GetUncompressedLength(_compressedData[i].data(), _compressedData[i].size(), &partialSize);

            //totalUncompressedBytes += partialSize;

            snappy::Uncompress(_compressedData[i].data(), _compressedData[i].size(), &uncompressedData[i]);

            //blocksSizes[i] = partialSize;

            //std::cout << "Uncompressed block size in bytes is : " << blocksSizes[i] << std::endl;

        }

        //_ddata = new T[totalUncompressedBytes];
        //_ddata = new T[nblocks * blockSize * _ncols * size(T)];

        int ddataOffset = 0;

        for (int i = 0 ; i < nBlocks; i++){

           
            //T* block = new T[blocksSizes[i]];
            
            T* block = new T[_blockSizeRows * _ncols * sizeof(T)];

            block = (T*)uncompressedData[i].c_str();

            for (int j = ddataOffset; j < ddataOffset + _blockSizeRows*_ncols ; j++){
                
                _ddata[j] = block[j - i * ddataOffset];

            }

            if (ddataOffset < totalUncompressedBytes / sizeof(T)){

                ddataOffset += _blockSizeRows*_ncols;

            }
        }

        // collect garbage 
        //delete [] block;
        //delete [] uncompressedData;

        _denseAllocated = true;

        if (removeCompressed){

            _isCompressed = false; 
            delete [] _compressedData; 

        }

    }

}



//TODO: printing a matrix using cout ? 
//std::ostream & operator << (std::ostream & o, const mat<T> & m){



//}
