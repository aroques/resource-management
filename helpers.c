#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "helpers.h"

char** split_string(char* str, char* delimeter) {
    char* substr;
    char** strings = malloc(10 * sizeof(char));

    substr = strtok(str, delimeter);

    int i = 0;
    while (substr != NULL)
    {
        strings[i] = malloc(20 * sizeof(char));
        strings[i] = substr;
        substr = strtok(NULL, delimeter);
        i++;
    }

    return strings;

}

char* get_timestamp() {
    char* timestamp = malloc(sizeof(char)*10);
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    sprintf(timestamp, "%d:%d:%d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    return timestamp;
}

void parse_cmd_line_args(int argc, char* argv[]) {
    int option;
    while ((option = getopt (argc, argv, "h")) != -1)
    switch (option) {
        case 'h':
            print_usage();
            break;
        default:
            print_usage();
    }
}

void print_usage() {
    fprintf(stderr, "Usage: oss\n");
    exit(0);
}

void set_timer(int duration) {
    struct itimerval value;
    value.it_interval.tv_sec = duration;
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    if (setitimer(ITIMER_REAL, &value, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
}

bool event_occured(unsigned int pct_chance) {
    unsigned int percent = (rand() % 100) + 1;
    if (percent <= pct_chance) {
        return 1;
    }
    else {
        return 0;
    }
}