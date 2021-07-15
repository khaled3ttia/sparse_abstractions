CC=clang++
HEADERDIR=/home/khaled/Documents/anl/snappy/
SNAPPYBUILD=/home/khaled/Documents/anl/snappy/build
CFLAGS=-std=c++11 -O3 -flto -march=native -g -I $(HEADERDIR) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD) 
SFLAGS=-std=c++11 -O1 -g -fsanitize=address -fno-omit-frame-pointer -I $(HEADERDIR) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD)
nocompress:
	$(CC) $(CFLAGS) -DNOCOMPRESS bench.cpp -lsnappy -o bench_nocompress.exe
compress:
	$(CC) $(CFLAGS) bench.cpp -lsnappy -o bench_compress.exe
compress_mmap:
	$(CC) $(CFLAGS) -DUSE_MMAP bench.cpp -lsnappy -o bench_compress_mmap.exe
nocompress_mmap:
	$(CC) $(CFLAGS) -DUSE_MMAP -DNOCOMPRESS  bench.cpp -lsnappy -o bench_nocompress_mmap.exe
sanitize:
	$(CC) $(SFLAGS) bench.cpp -lsnappy -o bench_sanitize.exe
clean:
	rm *.exe
