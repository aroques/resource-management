#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>

char** split_string(char* str, char* delimeter);
char* get_timestamp();
void print_usage();
void parse_cmd_line_args(int argc, char* argv[]);
void set_timer(int duration);
bool event_occured(unsigned int pct_chance);

#endif
