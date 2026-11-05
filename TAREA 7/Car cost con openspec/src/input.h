#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

#include "parse.h"

#define input_trim parse_trim
#define input_parse_double parse_double
#define input_parse_int parse_int

int input_parse_yes_no(const char *text, int *value);

int input_read_line(char *buffer, size_t size);
int input_read_text(const char *prompt, char *buffer, size_t size);
int input_read_text_optional(const char *prompt, const char *current, char *buffer, size_t size);
int input_read_double(const char *prompt, double *value);
int input_read_double_optional(const char *prompt, double current, double *value, int *changed);
int input_read_int_range(const char *prompt, int minimum, int maximum, int *value);
int input_read_yes_no(const char *prompt, int *value);

#endif
