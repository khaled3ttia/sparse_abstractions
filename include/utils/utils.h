#ifdef USE_MMAP
class sbuffer:public std::streambuf {
    public:
        sbuffer(char* start, size_t size){
            this->setg(start, start, start+size);
        }
};
#endif 

double timeit(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ( (double) tv.tv_sec + (double) tv.tv_usec * 1.e-6);
}


