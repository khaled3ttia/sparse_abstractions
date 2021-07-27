#ifndef MAT_H
#define MAT_H

#include <sys/time.h>
#include <iostream>
#include <string>
#include <sstream>

#ifndef USE_MMAP
#include <fstream>
#endif


#ifdef USE_MMAP
#include <streambuf>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <assert.h>
#include <cmath>
#include "snappy.h"
#include "blockcache.h"
#include "utils.h"

enum format {
    DENSE,
    COO
};

enum compressMode {
    ELEMENTS,
    ROWS
};

template <typename T>
class mat {

    int _id;
    
    // common fields
    unsigned int _ncols;
    unsigned int _nrows; 
    unsigned int _nnz;
    enum format _mtFormat;

    // structures for dense
    T * _ddata;
    std::string * _compressedData; 

    bool _denseAllocated = false;

   
    // structures for COO 
    T * _coovals;
    int * _cooRowIdx; 
    int * _cooColIdx; 
    bool _cooAllocated = false; 

    
    // compression related
    bool _isCompressed = false;
    enum compressMode _cmode = ELEMENTS;
    size_t _blockSize = 2000;
    bool _useCompressed = false;

    void compressByRow(bool = true);
    void compressByElement(bool = true);

    void decompressByRow(bool = true);

    T& getCompressedElement(int, int);

    public:
    
    static BlockCache<T> _Cache;
    static int _nMatrices;
    static int _DecompressCallsCount;
    static double _DecompressTime;
    static double _InsertTime;
    static void setCacheSize(int);
    
    mat();
    
    mat(int, int = 1, format=DENSE);   


    mat(int, int, int, T *&);

    //mat(int, int, int, T *&, int *&, int *&);
    
    // Read a matrix from a mtx file using the specified format 
    mat(const char*, int=2,  format=DENSE, bool=true, bool=false, compressMode=ELEMENTS);
   

    // copy constructor 
    mat(const mat &);

    // Overloading the ( ) operator to access matrix 
    T& operator () (int, int=0);

    // Overloading the [ ] operate to access matrix using a flat index
    T& operator [] (int);

    mat operator + (const mat &);

    mat operator - (const mat &);

    mat operator * (const T);


    // Copy operator 
    mat & operator = (const mat &);


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
