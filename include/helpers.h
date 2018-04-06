#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>

char** split_string(char* str, char* delimeter);
void free_strings(char** arr, int size);
char* get_timestamp();
void print_usage();
void parse_cmd_line_args(int argc, char* argv[]);
void set_timer(int duration);
bool event_occured(unsigned int pct_chance);
unsigned int** create_array(int m, int n);
void destroy_array(unsigned int** arr);
void print_and_write(char* str, FILE* fp);

#endif
