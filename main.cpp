#include "mat.h"


int main(){
    

    /*
        
    mat<float> b(10);

    std::cout << b(9) << std::endl;

    
    mat<float> c(5);

    // Test copy operator
    c = b; 

    // Test invalid mtx file name
    mat<float> d("Hello", DENSE);
    
   
    // Test copy constructor
    mat<float> x(b);


    // Test constructor with mtx file
    mat<float> Y("s.mtx", COO);
    */

    mat<float> smtx("s.mtx", DENSE);

    // Test ( ) operator overload 
    std::cout << smtx(7,4) << std::endl;


    /*
    // Test format conversion

    smtx.denseToCoo();

   
    std::cout << "after double conversion" << std::endl;

    std::cout << smtx(7,4) << std::endl;

    // Test setting through the ( ) operator

    smtx(7,4) = 5;

    std::cout << smtx(7,4) << std::endl;
    

    // Test overloaded addition of two matrices
    mat<float> g = smtx  + smtx;


    std::cout << g(7,4) << std::endl;
    */
}
