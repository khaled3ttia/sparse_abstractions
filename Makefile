CC=clang++
HEADERDIR=/home/khaled/Documents/anl/snappy/
SNAPPYBUILD=/home/khaled/Documents/anl/snappy/build
CFLAGS=-std=c++11 -O3 -flto -march=native -g -I $(HEADERDIR) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD) 
nocompress:
	$(CC) $(CFLAGS) -DNOCOMPRESS bench.cpp -lsnappy -o bench_nocompress.exe
compress:
	$(CC) $(CFLAGS) bench.cpp -lsnappy -o bench_compress.exe
clean:
	rm *.exe
