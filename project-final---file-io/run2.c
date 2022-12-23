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

float read_file(int in, int block_size, int block_count) {

    unsigned int *buf = (unsigned int *)malloc(block_size);
    unsigned int res = 0;
    int cntr = 0;
    int temp;

    memset(buf, 0, block_size);
    
    clock_t start = clock();
    while ((temp = read(in, buf, block_size)) > 0) {
        res ^= xorBuf(buf, temp / 4);
        if (cntr == block_count) {
            break;
        }
        cntr += 1;
    }
    clock_t finish = clock();

    xor_res = res;
    memset(buf, 0, block_size);

    float runtime = (float)(finish - start)/CLOCKS_PER_SEC;
    float read_mb = 1;
    read_mb *= block_size;
    read_mb /= 1000000;
    read_mb *= block_count;

    printf("block_size: %d \t block_count: %d \t runtime: %f \t MB/s: %f\n", block_size, block_count, runtime, read_mb / runtime);
    return runtime;
   
}

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        printf("Usage: ./run2 <filename> <block_size>");
        return 0;
    }

    char* filename = argv[1];
    const size_t block_size = atoi(argv[2]);

    int in = open(filename, O_RDONLY);
    float timer = 0;
    float cntr = 1;

    stat(filename, &st);
    long long size = st.st_size;
    while (true) {
        timer = read_file(in,block_size, cntr);
        if (timer > 5.00) {
            break;
        }
        cntr *= 2;
        // printf("%lld %f %f\n", size, block_size * cntr, timer);
        if (block_size * cntr > size) {
            printf("large file is required to find reasonable file size");
            break;
        }
    }

    close(in);
 
    return xor_res;
}