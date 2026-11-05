#include "input.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

int input_parse_yes_no(const char *text, int *value)
{
    char buffer[32];
    char *cursor;

    if (text == NULL || value == NULL) {
        return 0;
    }
    snprintf(buffer, sizeof(buffer), "%s", text);
    parse_trim(buffer);
    for (cursor = buffer; *cursor != '\0'; cursor++) {
        *cursor = (char)tolower((unsigned char)*cursor);
    }
    if (strcmp(buffer, "s") == 0 || strcmp(buffer, "si") == 0
        || strcmp(buffer, "y") == 0 || strcmp(buffer, "yes") == 0) {
        *value = 1;
        return 1;
    }
    if (strcmp(buffer, "n") == 0 || strcmp(buffer, "no") == 0) {
        *value = 0;
        return 1;
    }
    return 0;
}

int input_read_line(char *buffer, size_t size)
{
    size_t length;

    if (buffer == NULL || size == 0) {
        return 0;
    }
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] != '\n') {
        int character;
        while ((character = getchar()) != '\n' && character != EOF) {
            /* descartar el resto de una linea demasiado larga */
        }
    } else if (length > 0) {
        buffer[length - 1] = '\0';
    }
    parse_trim(buffer);
    return 1;
}

int input_read_text(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);
    return input_read_line(buffer, size);
}

int input_read_text_optional(const char *prompt, const char *current, char *buffer, size_t size)
{
    printf("%s [%s]: ", prompt, current);
    fflush(stdout);
    if (!input_read_line(buffer, size)) {
        return 0;
    }
    if (buffer[0] == '\0') {
        snprintf(buffer, size, "%s", current);
    }
    return 1;
}

int input_read_double(const char *prompt, double *value)
{
    char buffer[256];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!input_read_line(buffer, sizeof(buffer))) {
            return 0;
        }
        if (parse_double(buffer, value)) {
            return 1;
        }
        printf("Entrada invalida. Escriba un numero.\n");
    }
}

int input_read_double_optional(const char *prompt, double current, double *value, int *changed)
{
    char buffer[256];

    for (;;) {
        printf("%s [%.2f]: ", prompt, current);
        fflush(stdout);
        if (!input_read_line(buffer, sizeof(buffer))) {
            return 0;
        }
        if (buffer[0] == '\0') {
            *value = current;
            *changed = 0;
            return 1;
        }
        if (parse_double(buffer, value)) {
            *changed = 1;
            return 1;
        }
        printf("Entrada invalida. Escriba un numero o deje vacio para conservar.\n");
    }
}

int input_read_int_range(const char *prompt, int minimum, int maximum, int *value)
{
    char buffer[128];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!input_read_line(buffer, sizeof(buffer))) {
            return 0;
        }
        if (parse_int(buffer, value) && *value >= minimum && *value <= maximum) {
            return 1;
        }
        printf("Entrada invalida. Escriba un numero entre %d y %d.\n", minimum, maximum);
    }
}

int input_read_yes_no(const char *prompt, int *value)
{
    char buffer[32];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!input_read_line(buffer, sizeof(buffer))) {
            return 0;
        }
        if (input_parse_yes_no(buffer, value)) {
            return 1;
        }
        printf("Responda 's' o 'n'.\n");
    }
}
