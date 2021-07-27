#include "mat.h"
#include <string.h>
#include <unistd.h>

void usage() {
  std::cout << "Options: " << std::endl;

  std::cout << " -f <filename> : file path of input MTX file" << std::endl;
  std::cout << " -c <cacheSize> : size of the software cache (in blocks)"
            << std::endl;
  std::cout << " -b <blockSize> : number of elements or rows (depending on "
               "compression mode) per block"
            << std::endl;
  std::cout << " -p : prints blockcache summary" << std::endl;
  std::cout << " -n : number of iterations for the benchmark" << std::endl;
  std::cout
      << " -m : compression mode\n\t0: compress by elements\n\t1:Compress by "
         "row\n\tthis affects whether -b <blocksize> is interpreted as number "
         "of elements or number of rows"
      << std::endl;
  std::cout << " -h : prints usage information" << std::endl;
  std::cout << "Example: " << std::endl;
  std::cout << "./bench.exe -f data/fidap001.mtx -c 32 - b 2" << std::endl;
}

int main(int argc, char **argv) {

  int opt;
  static const char *filename;
  static bool farg, carg, barg, narg, parg;
  static unsigned int cacheSize;
  static unsigned int blockSize;
  static unsigned int N, M;

  // Reading command line arguments
  while ((opt = getopt(argc, argv, "f:c:b:n:m:hp")) != -1) {
    switch (opt) {
    case 'f':
      if (optarg) {

        filename = optarg;
        farg = true;
      }
      break;
    case 'c':
      if (optarg) {

        cacheSize = atoi(optarg);
        carg = true;
      }
      break;
    case 'b':
      if (optarg) {
        blockSize = atoi(optarg);
        barg = true;
      }
      break;
    case 'n':
      if (optarg) {
        N = atoi(optarg);
        narg = true;
      }
      break;
    case 'm':
      if (optarg) {
        M = atoi(optarg);
      }
      break;
    case 'p':
      parg = true;
      break;
    case 'h':
      usage();
      break;
    }
  }

  // Setting default values
  if (!farg) {
    filename = "data/saylr3.mtx";
  }
  if (!carg) {
    cacheSize = 32;
  }
  if (!barg) {
    blockSize = 1000;
  }
  if (!narg) {
    N = 5;
  }

  enum compressMode cmode = (M == 1) ? ROWS : ELEMENTS;

  double *t = new double[N];
  memset(t, 0, N * sizeof(*t));

#ifdef DEBUG

  std::cout << "MTX file path: " << filename << std::endl;
  std::cout << "Compression Mode: " << ((M == 0) ? " by elements" : " by rows")
            << std::endl;
  std::cout << "Cache size: " << cacheSize << " blocks" << std::endl;
  std::cout << "Block Size: " << blockSize << ((M == 0) ? " elements" : " rows")
            << " per block" << std::endl;

#ifdef NOCOMPRESS
  std::cout << "NOTE: Compressed Matrix is not going to be used in "
               "calculation! Cache size has no effect!"
            << std::endl;
#endif

#endif

  mat<double>::setCacheSize(cacheSize);

#ifdef DEBUG
  std::cout << "Loading matrix..." << std::endl;
#endif

#ifdef NOCOMPRESS
  mat<double> smtx(filename, blockSize, DENSE, true, false, cmode);
#else
  mat<double> smtx(filename, blockSize, DENSE, true, true, cmode);
#endif

#ifdef DEBUG
  std::cout << "Number of rows : " << smtx.getNRows() << std::endl;
  std::cout << "Number of cols: " << smtx.getNCols() << std::endl;
  std::cout << "Running Benchmark .... " << std::endl;
#endif
  int nrows = smtx.getNRows();
  int ncols = smtx.getNCols();

  ssize_t nelements = nrows * ncols;
  double bytes = sizeof(double) * nelements;
  ssize_t j;
  for (int i = 0; i < N; i++) {
    volatile double result = 0.;
    t[i] = timeit();
    for (j = 0; j < nrows; j++) {
      ssize_t k;
      for (k = 0; k < ncols; k++) {
        result = result + smtx(j, k);
      }
    }
    t[i] = timeit() - t[i];

    // std::cout << "Result : " << result << std::endl;
    //
#ifdef DEBUG
    if (i == N - 1) {
      if (parg) {
        mat<double>::_Cache.printInfo();
      }
    }

#endif
    // mat<double>::_Cache.flushCache();
  }

  double avgTime = 0;
  for (int i = 0; i < N; i++) {
    avgTime = avgTime + t[i];
  }
  avgTime = avgTime / N;

  std::cout << "Average Execution time : " << avgTime << " seconds"
            << std::endl;
  std::cout << "Memory Bandwidth: " << bytes * 1.0E-06 / avgTime << " MB/s"
            << std::endl;

#ifdef DEBUG
  std::cout << "Number of calls to decompression function :"
            << mat<double>::_DecompressCallsCount << std::endl;

  std::cout << "Total time spent in decompression : "
            << mat<double>::_DecompressTime << std::endl;

  std::cout << "Total time spent in blockcache insertions: "
            << mat<double>::_InsertTime << std::endl;
#endif

  delete[] t;
}
