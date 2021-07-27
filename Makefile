CXX=clang++
HEADERDIR=/home/khaled/snappy/
SNAPPYBUILD=/home/khaled/snappy/build
CXXFLAGS=-std=c++11 -fuse-ld=lld -O3 -flto -march=native -g -I $(HEADERDIR) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD) 
SFLAGS=-std=c++11 -O1 -g -fsanitize=address -fno-omit-frame-pointer -I $(HEADERDIR) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD)
all: release debug
debug:
	$(CXX) $(CXXFLAGS) -DDEBUG -DUSE_MMAP bench.cpp -lsnappy -o debug_compress.exe
	$(CXX) $(CXXFLAGS) -DNOCOMPRESS -DDEBUG -DUSE_MMAP bench.cpp -lsnappy -o debug_nocompress.exe
release:
	$(CXX) $(CXXFLAGS) -DUSE_MMAP bench.cpp -lsnappy -o compress.exe
	$(CXX) $(CXXFLAGS) -DNOCOMPRESS -DUSE_MMAP bench.cpp -lsnappy -o nocompress.exe
sanitize:
	$(CC) $(SFLAGS) bench.cpp -lsnappy -o bench_sanitize.exe
clean:
	rm *.exe
