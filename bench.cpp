#include "mat.h"
#include <chrono>
#include <unistd.h>

void usage(){
    std::cout << "Options: " << std::endl;

    std::cout << " -f <filename> : file path of input MTX file" << std::endl;
    std::cout << " -c <cacheSize> : size of the software cache (in blocks)" << std::endl;
    std::cout << " -b <blockSize> : number of rows per block" << std::endl;
    std::cout << " -h : prints usgae information" << std::endl;

    std::cout << "Example: " << std::endl;
    std::cout << "./bench.exe -f data/fidap001.mtx -c 32 - b 2" << std::endl;
}

int main(int argc, char** argv) {

    int opt; 
    std::string filename;
    bool farg, carg,barg;
    unsigned int cacheSize; 
    unsigned int blockSize; 

    while ((opt = getopt(argc, argv, "f:c:b:h")) != -1){

        switch (opt){
            case 'f':
                if (optarg){

                    filename = optarg;
                    farg = true;
                }
                break;
            case 'c':
                if (optarg){

                    cacheSize = atoi(optarg);
                    carg = true;
                }
                break;
            case 'b':
                if (optarg){


                    blockSize = atoi(optarg);
                    barg = true;
                }
                break;
            case 'h':
                usage();
                break;
        }

    }

    if (!farg){
        filename = "data/fidap011.mtx";        
    }
    if (!carg){
        cacheSize = 32;
    }
    if (!barg){
        blockSize = 2; 
    }

    std::cout << "MTX file path: " << filename << std::endl;
    std::cout << "Cache size: " << cacheSize << " blocks" << std::endl;
    std::cout << "Block Size (Rows per block) : " << blockSize << std::endl;


    float result = 0;

    mat<float>::setCacheSize(cacheSize);
    

    std::cout << "Loading matrix..." << std::endl;
    mat<float> smtx(filename , blockSize, DENSE);

    std::cout << "Number of rows : " << smtx.getNRows() << std::endl;
    std::cout << "Number of cols: " << smtx.getNCols() << std::endl;

    std::cout << "Running Benchmark .... " << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < smtx.getNRows() ; i++){

        for (int j = 0 ; j < smtx.getNCols(); j++){
            result += smtx(i, j);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();


    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end- start).count();

    std::cout << "Execution time : " << duration << " seconds" <<  std::endl;
}
