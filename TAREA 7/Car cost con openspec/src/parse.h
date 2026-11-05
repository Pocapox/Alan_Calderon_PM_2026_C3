#ifndef PARSE_H
#define PARSE_H

#include <stddef.h>

void parse_trim(char *text);
int parse_double(const char *text, double *value);
int parse_int(const char *text, int *value);

#endif
