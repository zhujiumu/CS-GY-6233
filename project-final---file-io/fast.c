#include <stdio.h> 
#include <fcntl.h> 
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
struct stat st;

unsigned int xor_res;

unsigned int xorBuf(unsigned int *buf, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result ^= buf[i];
    }
    return result;
}

int read_file(int in, int block_size, int block_count) {

    unsigned int *buf = (unsigned int *)malloc(block_size);
    unsigned int res = 0;
    int temp;

    memset(buf, 0, block_size);
    clock_t start = clock();
    int cntr = 1;
    while ((temp = read(in, buf, block_size)) > 0) {
        res ^= xorBuf(buf, temp / 4);
        cntr += 1;
    }
    clock_t finish = clock();

    xor_res = res;
    memset(buf, 0, block_size);

    float runtime = (float)(finish - start)/CLOCKS_PER_SEC;
    float read_mb = 1;
    read_mb *= block_size;
    read_mb /= 1000000;
    read_mb *= cntr;


    printf("xor result: %d \t MB/s:%f \t runtime: %f\n", xor_res, read_mb / runtime, runtime);
    return runtime;
   
}

int main(int argc, char *argv[]) {
    
    if (argc != 2) {
        printf("Usage: ./fast <filename>");
        return 0;
    }

    char* filename = argv[1];
    const size_t block_size = 262144;

    int in = open(filename, O_RDONLY);
    float timer = 0;
    float cntr = 1;

    timer = read_file(in,block_size, cntr);
    // stat(filename, &st);
    // int size = st.st_size;
    // printf("%d", size);

    close(in);
 
    return xor_res;
}