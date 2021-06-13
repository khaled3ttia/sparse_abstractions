#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "snappy.h"

enum format {
    DENSE,
    COO
};


template <typename T>
class mat {
    
    // common fields
    int _ncols;
    int _nrows; 
    int _nnz;
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


    void compressByRow(size_t, bool);

    public:
    mat();
    
    mat(int, int = 1, format=DENSE);   


    mat(int, int, int, T *&);

    //mat(int, int, int, T *&, int *&, int *&);
    
    // Read a matrix from a mtx file using the specified format 
    mat(std::string, format);
   

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
