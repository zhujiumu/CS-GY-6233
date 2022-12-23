#include <stdio.h> 
#include <fcntl.h> 
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

const char garbage[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

int helper(int n) { 
    return rand() % n; 
}

char *randomString(int len) {
  char *res = malloc((len) * sizeof(char));
  int i;
  for (i = 0; i < len; i++) {
    res[i] = garbage[helper(strlen(garbage))];
  }
  return res;
}

unsigned int xorBuf(unsigned int *buf, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result ^= buf[i];
    }
    return result;
}

int main(int argc, char *argv[]) {
    
    if (argc != 5) {
        printf("Usage: filename -r|-w block_size block_count");
        return 0;
    }

    char* filename = argv[1];
    const size_t block_size = atoi(argv[3]);
    const size_t block_count = atoi(argv[4]);
    size_t target_size = block_count * block_size;
    bool read_bool = (strcmp(argv[2], "-r") == 0 || strcmp(argv[2], "-R") == 0);

    if (read_bool) {
        int in = open(filename, O_RDONLY);
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

        float runtime = (float)(finish - start)/CLOCKS_PER_SEC;
        
        float written_kb = target_size / 1000;
        float written_mb = written_kb / 1000;
        printf("xor result: %d \n", res);
        
        printf("%f KB(s) | %f MB(s) read from %s on disk.\n", written_kb, written_mb, filename);
        printf("runtime: %f seconds | KB/s: %f | MB/s: %f ", runtime, (float)written_kb / runtime, (float)written_mb / runtime);
        
    } else {
        int out = open(filename, O_RDWR|O_TRUNC, 0);
        if(out < 0) {
            out = creat(filename, S_IREAD | S_IWRITE | S_IRWXO | S_IRWXG);
        }
        char * buf = (char *)(malloc(block_size));

        srand(time(NULL));
        char* filler = randomString(block_size - 1);
        
        int winner;

        clock_t start = clock();

        for (int i = 0; i < block_count; i++) {
            buf = randomString(block_size);
            write(out, buf, strlen(buf));
        }

        clock_t finish = clock();

        float runtime = (float)(finish - start)/CLOCKS_PER_SEC;
        
        float written_kb = target_size / 1000;
        float written_mb = written_kb / 1000;

        printf("%f KB(s) | %f MB(s) written to %s on disk.\n", written_kb, written_mb, filename);
        printf("runtime: %f seconds | KB/s: %f | MB/s: %f ", runtime, (float)written_kb / runtime, (float)written_mb / runtime);
        close(out);

    };

    return 0;
}