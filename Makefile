CC=clang++
HEADERDIR=/home/khaled/Documents/anl/snappy/
SNAPPYBUILD=/home/khaled/Documents/anl/snappy/build
CFLAGS=-std=c++11 -g -I $(HEADERDIR) -I $(SNAPPYBUILD) -L $(SNAPPYBUILD) 
default:
	$(CC) $(CFLAGS) main.cpp -lsnappy -o testApp.exe
clean:
	rm *.exe
