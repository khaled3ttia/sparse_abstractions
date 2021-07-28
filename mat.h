#ifndef MAT_H
#define MAT_H

#include <iostream>
#include <sstream>
#include <string>
#include <sys/time.h>

#ifndef USE_MMAP
#include <fstream>
#endif

#ifdef USE_MMAP
#include <fcntl.h>
#include <streambuf>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "blockcache.h"
#include "snappy.h"
#include "utils.h"
#include <assert.h>
#include <cmath>

enum format { DENSE, COO };

enum compressMode { ELEMENTS, ROWS };

template <typename T> class mat {

  std::string *_compressedData; 
  T *_ddata; 
  T *_coovals;
  float _invBlockSize;

  int _id;
  int *_cooRowIdx;
  int *_cooColIdx;
  unsigned int _ncols;
  unsigned int _nrows;
  unsigned int _nnz;
  size_t _blockSize;

  enum format _mtFormat;
  enum compressMode _cmode = ELEMENTS;
 
  bool _denseAllocated = false;
  bool _cooAllocated = false;
  bool _isCompressed = false;
  bool _useCompressed = false;
  
  // this flag will be = (_useCompressed && _isCompressed)
  bool _compElems = false;

  void compressByRow(bool = true);
  void compressByElement(bool = true);

  // method called to access an element when _compElems==true
  T& getCompressedElement(int, int);

public:

  mat() = delete;

  // Read a matrix from a mtx file using the specified format
  mat(const char *, int = 2, format = DENSE, bool = true, bool = false,
      compressMode = ELEMENTS);

  // copy constructor
  mat(const mat &);

  static BlockCache<T> _Cache;
  static double _DecompressTime;
  static double _InsertTime; 
  static int _nMatrices;
  static int _DecompressCallsCount; 
  static void setCacheSize(int);
  // Overloading the ( ) operator to access matrix
  T &operator()(int, int = 0);

  // Overloading the [ ] operate to access matrix using a flat index
  T &operator[](int);

  mat operator+(const mat &);

  mat operator-(const mat &);

  mat operator*(const T);

  // Copy operator
  mat &operator=(const mat &);

  int getNRows();
  int getNCols();
  int getNNnz();

  // Destructor
  ~mat();

  // Conversion between formats
  // TODO: move to private since user shouldn't have access to
  // (public now for testing)

  void cooToDense();

  void denseToCoo();
};

// Implementation
#include "mat_imp.h"

#endif
