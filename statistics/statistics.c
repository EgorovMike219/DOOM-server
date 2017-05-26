#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "statistics.h"


void record_in_statistics(int id, int health) {
    char filename[60];
    int success_record = sprintf(filename, "%d %s", id, ".txt");
    if(success_record == 0) {
        fprintf(stderr, "Can\'t record in file\n");
        exit(-1);
    }
    else {
        FILE *f = fopen(filename, "w");
        if (f == NULL) {
            fprintf(stderr, "Can\'t open file for record\n");
            exit(-1);
        }
        else {
            if (health < 0) {
                fprintf(f, "id = %d died\n", id);
            }
            else {
                fprintf(f, "user id = %d, health = %d ", id, health);
            }
        }
        fclose(f);
    }
}

void answer_from_statistics(int id) {
    char filename[60];
    char answer[100];
    int success_reading = sprintf(filename, "%d %s", id, ".txt");
    if(success_reading == 0) {
        fprintf(stderr, "Can\'t read from file\n");
        exit(-1);
    }
    else {
        FILE *f = fopen(filename, "r");
        if (f == NULL) {
            fprintf(stderr, "Can\'t open file for reading\n");
            exit(-1);
        }
        else if (f < 0) {
            fprintf(stderr, "File does not exist\n");
            exit(-1);
        }
        else {
            fgets(answer,100,f);
            puts(answer);
        }
        fclose(f);
    }
}
