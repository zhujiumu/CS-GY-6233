#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"


int main(int argc, char *argv[]) {

    int priorty, pid;

    if (argc < 3 || argc > 3) {
        printf(2, "Usage: nice pid priority\n");
        exit();
    }

    pid = atoi(argv[1]);
    priorty = atoi(argv[2]);
    if (priorty < 0 || priorty > 20) {
        printf(2, "Invalid priority (0 - 20)!\n");
        exit();
    }

    chpr(pid, priorty);
    exit();
}
