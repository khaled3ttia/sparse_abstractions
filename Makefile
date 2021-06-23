CC=clang++
HEADERDIR=/home/khaled/Documents/anl/snappy/
SNAPPYBUILD=/home/khaled/Documents/anl/snappy/build
CFLAGS=-std=c++11 -O3 -flto -march=native -g -I $(HEADERDIR) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD) 
default:
	$(CC) $(CFLAGS) bench.cpp -lsnappy -o bench.exe
clean:
	rm *.exe
