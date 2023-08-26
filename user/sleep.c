#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(2, "ERROR: sleep time required\n");
        exit(1);
    }
    sleep(atoi (argv[1]));
    printf("在睡觉");
    exit(0);
}