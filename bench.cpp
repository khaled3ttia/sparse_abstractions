#include "mat.h"
#include <sys/time.h>
#include <unistd.h>


double timeit(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ( (double) tv.tv_sec + (double) tv.tv_usec * 1.e-6);
}

void usage(){
    std::cout << "Options: " << std::endl;

    std::cout << " -f <filename> : file path of input MTX file" << std::endl;
    std::cout << " -c <cacheSize> : size of the software cache (in blocks)" << std::endl;
    std::cout << " -b <blockSize> : number of rows per block" << std::endl;
    std::cout << " -h : prints usgae information" << std::endl;
    std::cout << "Example: " << std::endl;
    std::cout << "./bench.exe -f data/fidap001.mtx -c 32 - b 2" << std::endl;
}

static double t[5] = {0};


int main(int argc, char** argv) {

    int opt; 
    static std::string filename;
    static bool farg,carg,barg, compress = true;
    static unsigned int cacheSize; 
    static unsigned int blockSize; 


    // Reading command line arguments
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


    // Setting default values
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
#ifdef NOCOMPRESS 
    std::cout << "No compression to be done (Cache size and block size are ignored)!" << std::endl;
#endif

    mat<double>::setCacheSize(cacheSize);
    

    std::cout << "Loading matrix..." << std::endl;

#ifdef NOCOMPRESS
    mat<double> smtx(filename, blockSize, DENSE, true, false);
#else
    mat<double> smtx(filename, blockSize, DENSE, true, true);
#endif


    std::cout << "Number of rows : " << smtx.getNRows() << std::endl;
    std::cout << "Number of cols: " << smtx.getNCols() << std::endl;

    std::cout << "Running Benchmark .... " << std::endl;

           
    int nrows = smtx.getNRows();
    int ncols = smtx.getNCols();

    ssize_t nelements = nrows * ncols; 
    double bytes = sizeof(double) * nelements;
    ssize_t j;
    for (int i = 0 ; i < 5 ; i++){
        double result = 0.;
        t[i] = timeit();
        for (j  = 0 ; j < nelements; j++){
            result = result +  smtx[j];
        } 
        t[i] = timeit() - t[i];    
        std::cout << "Result : " << result << std::endl;

        mat<double>::_Cache.flushCache();
    }

    double avgTime = 0;
    for (int i = 0; i < 5 ; i++){
        avgTime = avgTime + t[i];
    }
    avgTime = avgTime / 5;

    std::cout << "Average Execution time : " << avgTime << " seconds" << std::endl;
    std::cout << "Memory Bandwidth: " << bytes * 1.0E-06 / avgTime << " MB/s" << std::endl;
}
