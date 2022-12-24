#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"

#ifndef MYVARIABLE
#define MYVARIABLE 1
#endif

unsigned int xorshift(unsigned int state[]) {
  unsigned int x = state[0];
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  state[0] = x;
  return x;
}


unsigned int x = 548787455, y = 842502087, z = 3579807591, w = 273326509;

unsigned int xorshift2() {
	unsigned int t = x ^ (x << 11);
	x = y; y = z; z = w;
	return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}
int main(int argc, char *argv[]) {

    int min = 20;
    int max = 0;
    int tot = 0;
    int cntr = 0;
    //unsigned int state[1] = {1234};  // "seed" (can be anthing but 0)
    
    for (int i = 0; i < 1000; i++) {
        int a = xorshift2();
        if (a < min) {
            min = a;
        }
        if (a > max) {
            max = a;
        }
        tot += a;
        cntr += 1;
    }

    printf(1, "min: %d\nmax: %d\navg: %d\nnumber of runs: %d\n", min, max, tot / 1000, cntr);
    return 0;
}
