template <typename T> BlockCache<T> mat<T>::_Cache(128);

template <typename T> int mat<T>::_nMatrices;

template <typename T> int mat<T>::_DecompressCallsCount = 0;

template <typename T> double mat<T>::_DecompressTime = 0.0;

template <typename T> double mat<T>::_InsertTime = 0.0;

template <typename T> void mat<T>::setCacheSize(int val) {
  _Cache.setMaxSize(val);
}

// Dummy default constructor, if no parameters passed
// create a 10-element array
template <typename T> mat<T>::mat() {

  _id = _nMatrices;
  _nMatrices++;

  _ddata = new T[10];
  _denseAllocated = true;
  _mtFormat = DENSE;
  for (int i = 0; i < 10; i++) {
    _ddata[i] = i;
  }
}

// Constructor that uses only number of rows, number of columns and matrix
// format
template <typename T>
mat<T>::mat(int nrows, int ncols, format mtFormat)
    : _nrows(nrows), _ncols(ncols), _mtFormat(mtFormat) {

  _id = _nMatrices;
  _nMatrices++;

  int nElems = nrows * ncols;

  if (_mtFormat == DENSE) {

    _ddata = new T[nElems];

    for (int i = 0; i < nElems; i++) {
      _ddata[i] = i;
    }

    _denseAllocated = true;

  } // TODO (if _mtFormat == COO)
}

// Constructor for dense
template <typename T>
mat<T>::mat(int nrows, int ncols, int nnz, T *&data)
    : _nrows(nrows), _ncols(ncols), _nnz(nnz) {

  _id = _nMatrices;
  _nMatrices++;

  _mtFormat = DENSE;

  _ddata = new T[_nrows * _ncols];

  _ddata = data;

  _denseAllocated = true;
}

/*
// Constructor for COO
template <typename T>
mat<T>::mat(int nrows, int ncols, int nnz, T *& coovals, int *& cooRowIdx, int
*& cooColIdx): _nrows(nrows), _ncols(ncols), _nnz(nnz){

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
mat<T>::mat(const char *matFile, int blockSize, format mtFormat, bool compress,
            bool useCompressed, compressMode cmode)
    : _blockSize(blockSize), _invBlockSize(1.0 / blockSize),
      _mtFormat(mtFormat), _useCompressed(useCompressed), _cmode(cmode) {

  double startTime = timeit();

  _id = _nMatrices;

  int currentLine = 0;
  int currentIndex = 0;

  std::string line;
#ifdef USE_MMAP
  int fd = open(matFile, O_RDONLY);
  if (fd == -1) {
    std::cerr << "Cannot open file!" << std::endl;
    exit(-1);
  }

  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    std::cerr << "Cannot get file info!" << std::endl;
    exit(-1);
  }

  size_t fileLen = sb.st_size;

  // mmap file
  char *filePtr =
      static_cast<char *>(mmap(NULL, fileLen, PROT_READ, MAP_PRIVATE, fd, 0u));

  if (filePtr == MAP_FAILED) {
    std::cerr << "Unable to memory map file!" << std::endl;
    exit(-1);
  }

  // streambuffer to handle mapped region
  sbuffer buffer(filePtr, fileLen);
  std::istream in(&buffer);
#endif

#ifndef USE_MMAP
  std::ifstream in(matFile);

  if (!in) {
    // file does not exist
    std::cerr << "Cannot open file!" << std::endl;
    exit(-1);
  }

  if (in.is_open()) {
#endif
    while (std::getline(in, line)) {

      std::istringstream iss(line);

      if (currentLine == 0) {
        currentLine++;
        continue;

      } else if (currentLine == 1) {

        if (!(iss >> _nrows >> _ncols >> _nnz)) {
          return;
        }

        if (_mtFormat == COO) {

          if (_cooAllocated) {

            delete[] _coovals;
            delete[] _cooRowIdx;
            delete[] _cooColIdx;
          }

          _coovals = new T[_nnz];
          _cooRowIdx = new int[_nnz];
          _cooColIdx = new int[_nnz];
          _cooAllocated = true;

        } else if (_mtFormat == DENSE) {

          if (_denseAllocated) {

            delete[] _ddata;
          }

          int numElems = _ncols * _nrows;
          _ddata = new T[numElems];
          for (int i = 0; i < numElems; i++) {
            _ddata[i] = 0;
          }
          _denseAllocated = true;
        }

        currentLine++;

      } else {
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

          } else if (_mtFormat == DENSE) {
            _ddata[(rowIdx - 1) * _ncols + (colIdx - 1)] = val;
          }
          currentIndex++;
        }
        currentLine++;
      }
    }
#ifndef USE_MMAP
  }
#endif

  double duration = timeit() - startTime;

#ifdef USE_MMAP

  close(fd);

  int munmap_out = munmap(filePtr, fileLen);

#endif

  _nMatrices++;

#ifdef DEBUG
  std::cout << "Matrix loading time: " << duration << " seconds" << std::endl;
  std::cout << "Size before compression: "
            << _ncols * _nrows * sizeof(T) / (1e06) << " MB" << std::endl;
#endif

  if (compress) {
    if (_mtFormat == DENSE) {
      if (_cmode == ELEMENTS) {
        compressByElement(false);
      } else {
        compressByRow(false);
      }
    }
  }

  _compElems = _isCompressed && _useCompressed;
}

template <typename T>
mat<T>::mat(const mat<T> &rhs)
    : _ncols(rhs._ncols), _nrows(rhs._nrows), _nnz(rhs._nnz),
      _mtFormat(rhs._mtFormat) {

  _id = _nMatrices;
  if (_mtFormat == DENSE) {
    size_t numElems = _ncols * _nrows;

    if (!_denseAllocated) {

      _ddata = new T[numElems];
      _denseAllocated = true;

      for (int i = 0; i < numElems; i++) {

        _ddata[i] = rhs._ddata[i];
      }
    }
  } // TODO if (_mtFormat == COO)

  _nMatrices++;
}

template <typename T> T &mat<T>::getCompressedElement(int rowIdx, int colIdx) {

  if (_cmode == ELEMENTS) {

    int flatIdx = rowIdx * _ncols + colIdx;
    int elemWithinBlock = flatIdx % _blockSize;

    int bid = (int)(flatIdx * _invBlockSize);

    auto it = _Cache.find(bid);

    if (it != _Cache.getEnd()) {
      // Cache hit
      return _Cache[bid][elemWithinBlock];
    } else {
      double i_st = timeit();
      Block<T> newBlock{};
      _Cache.insert(bid, newBlock);
      _InsertTime = _InsertTime + (timeit() - i_st);

      _DecompressCallsCount++;

      double d_st = timeit();
      snappy::Uncompress(_compressedData[bid].data(),
                         _compressedData[bid].size(),
                         _Cache[bid].getDecompressedStr());
      _DecompressTime = _DecompressTime + (timeit() - d_st);

      _Cache[bid].setData();
      return _Cache[bid][elemWithinBlock];
    }

  } else {

    int blockId = floor(rowIdx / _blockSize);
    int rowWithinBlock = rowIdx % _blockSize;

    int bid = blockId;

    auto it = _Cache.find(bid);

    if (it != _Cache.getEnd()) {
      // Cache hit
      return _Cache[bid][rowWithinBlock * _ncols + colIdx];
    } else {
      Block<T> newBlock{};
      _Cache.insert(bid, newBlock);
      snappy::Uncompress(_compressedData[blockId].data(),
                         _compressedData[blockId].size(),
                         _Cache[bid].getDecompressedStr());
      _Cache[bid].setData();

      return _Cache[bid][rowWithinBlock * _ncols + colIdx];
    }
  }
}

// Overloading the () operator
template <typename T> T &mat<T>::operator()(int rowIdx, int colIdx) {

  // assert(rowIdx < _nrows && colIdx < _ncols);

  if (_mtFormat == COO) {
    cooToDense();
  }

  return _compElems ? getCompressedElement(rowIdx, colIdx)
                    : _ddata[rowIdx * _ncols + colIdx];
}

template <typename T> T &mat<T>::operator[](int flatIdx) {

  assert(flatIdx < _nrows * _ncols);
  if (__builtin_expect(_mtFormat == COO, 0)) {

    cooToDense();
  }
  if (_isCompressed && _useCompressed) {
    int rowIdx = flatIdx / _nrows;
    int colIdx = (flatIdx % _ncols);
    return getCompressedElement(rowIdx, colIdx);
  }
  return _ddata[flatIdx];
}

template <typename T> mat<T> mat<T>::operator+(const mat<T> &rhs) {

  if (!(_ncols == rhs._ncols && _nrows == rhs._nrows)) {

    std::cerr << "Cannot add incompatible matrices" << std::endl;

    // TODO fix this
    return *this;
  }

  if (_mtFormat == DENSE && rhs._mtFormat == DENSE) {

    int nnz = 0;
    int numElems = _ncols * _nrows;

    T *newdata = new T[numElems];

    for (int i = 0; i < numElems; i++) {

      newdata[i] = _ddata[i] + rhs._ddata[i];

      if (newdata[i] != 0) {

        nnz++;
      }
    }

    return mat<T>(_nrows, _ncols, nnz, newdata);

  } // TODO: other combinations of formats: COO, DENSE
}

template <typename T> mat<T> mat<T>::operator-(const mat<T> &rhs) {

  if (!(_ncols == rhs._ncols && _nrows == rhs._nrows)) {

    std::cerr << "Cannot add incompatible matrices" << std::endl;

    // TODO fix this
    return *this;
  }

  if (_mtFormat == DENSE && rhs._mtFormat == DENSE) {

    int nnz = 0;
    int numElems = _ncols * _nrows;

    T *newdata = new T[numElems];

    for (int i = 0; i < numElems; i++) {

      newdata[i] = _ddata[i] - rhs._ddata[i];

      if (newdata[i] != 0) {

        nnz++;
      }
    }

    return mat<T>(_nrows, _ncols, nnz, newdata);

  } // TODO: other combination of formats: COO, DENSE
}

template <typename T> mat<T> mat<T>::operator*(const T rhs) {

  if (_mtFormat == DENSE) {

    int numElems = _ncols * _nrows;

    int nnz = 0;

    T *newdata = new T[numElems];

    for (int i = 0; i < numElems; i++) {

      newdata[i] = _ddata[i] * rhs;

      if (newdata[i] != 0) {
        nnz++;
      }
    }

    return mat<T>(_nrows, _ncols, nnz, newdata);

  } else if (_mtFormat == COO) {

    if (rhs == 0) {

      std::cerr << "Multiplying a COO matrix by 0!" << std::endl;

      return *this;
    }

    T *newcoovals = new T[_nnz];

    for (int i = 0; i < _nnz; i++) {
      newcoovals[i] = _coovals[i] * rhs;
    }

    return mat<T>(_nrows, _ncols, _nnz, newcoovals, _cooRowIdx, _cooColIdx);
  }
}

// Overloading the = (copy operator)
template <typename T> mat<T> &mat<T>::operator=(const mat<T> &rhs) {

  if (this != &rhs) {
    _id = _nMatrices;
    _ncols = rhs._ncols;
    _nrows = rhs._nrows;
    _nnz = rhs._nnz;
    _mtFormat = rhs._mtFormat;

    if (_mtFormat == DENSE) {
      int numElems = _ncols * _nrows;
      if (!_denseAllocated) {

        _ddata = new T[numElems];
        _denseAllocated = true;
        for (int i = 0; i < numElems; i++) {

          _ddata[i] = rhs._ddata[i];
        }

      } else {
        delete[] _ddata;
        T *_newddata = new T[numElems];

        for (int i = 0; i < numElems; i++) {
          _newddata[i] = rhs._ddata[i];
        }

        _ddata = _newddata;
      }

    } // TODO (if _mtFormat == COO)

    _nMatrices++;
  }
  return *this;
}

template <typename T> mat<T>::~mat<T>() {

  if (_denseAllocated) {

    delete[] _ddata;
  } else if (_cooAllocated) {

    delete[] _coovals;
    delete[] _cooColIdx;
    delete[] _cooRowIdx;
  }

  if (_isCompressed) {
    delete[] _compressedData;
  }
}

template <typename T> void mat<T>::cooToDense() {

  if (!_cooAllocated) {

    std::cerr << "Matrix not in COO format!" << std::endl;
    return;
  }

  int numElems = _ncols * _nrows;
  if (!_denseAllocated) {

    _ddata = new T[numElems];

    _denseAllocated = true;
  }

  for (int i = 0; i < _nnz; i++) {

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

template <typename T> void mat<T>::denseToCoo() {

  if (!_denseAllocated) {

    std::cerr << "Matrix not in DENSE format!" << std::endl;
    return;
  }

  int numElems = _ncols * _nrows;

  // (if the matrix is not loaded from a .mtx file,
  // the value of _nnz will be 0)
  if (_nnz == 0) {

    for (int i = 0; i < _nrows; i++) {
      for (int j = 0; j < _ncols; j++) {

        if (this->operator()(i, j) != 0) {

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
    for (int i = 0; i < _nrows; i++) {

      for (int j = 0; j < _ncols; j++) {

        if (currentIndex >= _nnz) {

          break;
        }

        if (this->operator()(i, j) != 0 && currentIndex < _nnz) {

          _coovals[currentIndex] = this->operator()(i, j);
          _cooRowIdx[currentIndex] = i;
          _cooColIdx[currentIndex] = j;

          currentIndex++;
        }
      }

      if (currentIndex >= _nnz) {
        break;
      }
    }

    _cooAllocated = true;
    _mtFormat = COO;

    delete[] _ddata;
    _denseAllocated = false;
  }
}

template <typename T> void mat<T>::compressByRow(bool removeOriginal) {

  if (_mtFormat == DENSE) {

    if (!_denseAllocated) {

      std::cerr << "Data for matrix not allocated yet!" << std::endl;
      return;
    }

    // TODO (what if blockSize does not divide _nrows)

    int nBlocks = _nrows / _blockSize;
    _compressedData = new std::string[nBlocks];

    size_t compLength[nBlocks];
    size_t totalCompressed = 0;

    int blockOffset = 0;

    for (int i = 0; i < nBlocks; i++) {
      char *uncompressedBlock = (char *)(_ddata + blockOffset);
      compLength[i] =
          snappy::Compress(uncompressedBlock, _ncols * sizeof(T) * _blockSize,
                           &_compressedData[i]);

      blockOffset += _ncols * _blockSize;
      totalCompressed += compLength[i];
    }

    _isCompressed = true;
    if (removeOriginal) {
      delete[] _ddata;
      _denseAllocated = false;
    }

#ifdef DEBUG
    /*
    std::cout << "Size after compression: "
              << (double)(totalCompressed / (1e06)) << " MB" << std::endl;
              */
#endif
  }
}

template <typename T> void mat<T>::compressByElement(bool removeOriginal) {

  if (_mtFormat == DENSE) {

    if (!_denseAllocated) {

      std::cerr << "Data for matrix not allocated yet!" << std::endl;
      return;
    }

    // TODO (what if blockSize does not divide _nrows*_ncols)

    int nBlocks = ceil(_nrows * _ncols / _blockSize);

    _compressedData = new std::string[nBlocks];

    size_t compLength[nBlocks];
    size_t totalCompressed = 0;

    int blockOffset = 0;

    // T* block = new T[_ncols*_blockSizeRows];
    // T* block;
    for (int i = 0; i < nBlocks; i++) {

      // block = _ddata + blockOffset;

      // char * uncompressedBlock = (char*)block;

      char *uncompressedBlock = (char *)(_ddata + blockOffset);

      compLength[i] = snappy::Compress(
          uncompressedBlock, sizeof(T) * _blockSize, &_compressedData[i]);

      blockOffset += _blockSize;
      totalCompressed += compLength[i];
    }

    _isCompressed = true;

    if (removeOriginal) {

      delete[] _ddata;
      _denseAllocated = false;
    }
  }
}

template <typename T> int mat<T>::getNRows() { return _nrows; }

template <typename T> int mat<T>::getNCols() { return _ncols; }

template <typename T> int mat<T>::getNNnz() { return _nnz; }

template <typename T> void mat<T>::decompressByRow(bool removeCompressed) {

  // TODO: delete [] unused

  if (_mtFormat == DENSE) {

    if (!_isCompressed) {

      std::cerr << "Matrix is not compressed!" << std::endl;
      return;
    }

    int nBlocks = _nrows / _blockSize;

    size_t totalUncompressedBytes = 0;

    std::string *uncompressedData = new std::string[nBlocks];

    size_t blocksSizes[nBlocks];

    for (int i = 0; i < nBlocks; i++) {
      size_t partialSize;
      // snappy::GetUncompressedLength(_compressedData[i].data(),
      // _compressedData[i].size(), &partialSize);

      // totalUncompressedBytes += partialSize;

      snappy::Uncompress(_compressedData[i].data(), _compressedData[i].size(),
                         &uncompressedData[i]);

      // blocksSizes[i] = partialSize;

      // std::cout << "Uncompressed block size in bytes is : " << blocksSizes[i]
      // << std::endl;
    }

    //_ddata = new T[totalUncompressedBytes];
    //_ddata = new T[nblocks * blockSize * _ncols * size(T)];

    int ddataOffset = 0;

    for (int i = 0; i < nBlocks; i++) {

      // T* block = new T[blocksSizes[i]];

      T *block = new T[_blockSize * _ncols * sizeof(T)];

      block = (T *)uncompressedData[i].c_str();

      for (int j = ddataOffset; j < ddataOffset + _blockSize * _ncols; j++) {

        _ddata[j] = block[j - i * ddataOffset];
      }

      if (ddataOffset < totalUncompressedBytes / sizeof(T)) {

        ddataOffset += _blockSize * _ncols;
      }
    }

    // collect garbage
    // delete [] block;
    // delete [] uncompressedData;

    _denseAllocated = true;

    if (removeCompressed) {

      _isCompressed = false;
      delete[] _compressedData;
    }
  }
}

// TODO: printing a matrix using cout ?
// std::ostream & operator << (std::ostream & o, const mat<T> & m){

//}
