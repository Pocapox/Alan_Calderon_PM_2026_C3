#include "parse.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_trim(char *text)
{
    char *start;
    size_t length;

    if (text == NULL) {
        return;
    }
    start = text;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }
    length = strlen(text);
    while (length > 0 && isspace((unsigned char)text[length - 1])) {
        text[length - 1] = '\0';
        length--;
    }
}

int parse_double(const char *text, double *value)
{
    char buffer[256];
    char *end = NULL;
    char *cursor;
    double parsed;

    if (text == NULL || value == NULL) {
        return 0;
    }
    snprintf(buffer, sizeof(buffer), "%s", text);
    parse_trim(buffer);
    if (buffer[0] == '\0') {
        return 0;
    }
    for (cursor = buffer; *cursor != '\0'; cursor++) {
        if (*cursor == ',') {
            *cursor = '.';
        }
    }
    errno = 0;
    parsed = strtod(buffer, &end);
    if (end == buffer || errno == ERANGE || !isfinite(parsed)) {
        return 0;
    }
    while (*end != '\0' && isspace((unsigned char)*end)) {
        end++;
    }
    if (*end != '\0') {
        return 0;
    }
    *value = parsed;
    return 1;
}

int parse_int(const char *text, int *value)
{
    char buffer[128];
    char *end = NULL;
    long parsed;

    if (text == NULL || value == NULL) {
        return 0;
    }
    snprintf(buffer, sizeof(buffer), "%s", text);
    parse_trim(buffer);
    if (buffer[0] == '\0') {
        return 0;
    }
    errno = 0;
    parsed = strtol(buffer, &end, 10);
    if (end == buffer || errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }
    while (*end != '\0' && isspace((unsigned char)*end)) {
        end++;
    }
    if (*end != '\0') {
        return 0;
    }
    *value = (int)parsed;
    return 1;
}
