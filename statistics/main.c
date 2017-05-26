#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "statistics.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        printf ("Usage: dstats <player id> \n");
        exit(-1);
    }
    else {
        int parameter = atoi(argv[1]);
        answer_from_statistics(parameter);
    }
    return 0;
}
