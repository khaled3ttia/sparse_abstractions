CXX=clang++
SOURCE=sequential.cpp
SNAPPYHEADER=/home/khaled/Documents/anl/snappy/
SNAPPYBUILD=/home/khaled/Documents/anl/snappy/build
CXXFLAGS=-std=c++11 -fuse-ld=lld -O3 -flto -march=native -g -I $(SNAPPYHEADER) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD) 
SFLAGS=-std=c++11 -O1 -g -fsanitize=address -fno-omit-frame-pointer -I $(SNAPPYHEADER) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD)
all: release debug
debug:
	$(CXX) $(CXXFLAGS) -DDEBUG -DUSE_MMAP $(SOURCE) -lsnappy -o debug_compress.exe
	$(CXX) $(CXXFLAGS) -DNOCOMPRESS -DDEBUG -DUSE_MMAP $(SOURCE) -lsnappy -o debug_nocompress.exe
release:
	$(CXX) $(CXXFLAGS) -DUSE_MMAP $(SOURCE) -lsnappy -o compress.exe
	$(CXX) $(CXXFLAGS) -DNOCOMPRESS -DUSE_MMAP $(SOURCE) -lsnappy -o nocompress.exe
fillc:
	$(CXX) $(CXXFLAGS) -DUSE_MMAP -DFILL_COMPRESSED $(SOURCE) -lsnappy -o compress_fillc.exe
sanitize:
	$(CC) $(SFLAGS) $(SOURCE) -lsnappy -o bench_sanitize.exe
clean:
	rm *.exe
