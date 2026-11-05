#include "utf8.h"

#include <limits.h>

#ifdef _WIN32
int utf8_to_wide(const char *utf8, wchar_t *wide, size_t wide_count)
{
    int written;

    if (utf8 == NULL || wide == NULL || wide_count == 0 || wide_count > INT_MAX) {
        return 0;
    }
    written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, wide, (int)wide_count);
    if (written <= 0) {
        return 0;
    }
    wide[wide_count - 1] = L'\0';
    return 1;
}

int wide_to_utf8(const wchar_t *wide, char *utf8, size_t utf8_size)
{
    int written;

    if (wide == NULL || utf8 == NULL || utf8_size == 0 || utf8_size > INT_MAX) {
        return 0;
    }
    written = WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8, (int)utf8_size, NULL, NULL);
    if (written <= 0) {
        return 0;
    }
    utf8[utf8_size - 1] = '\0';
    return 1;
}
#else
#include <stdlib.h>
#include <string.h>

int utf8_to_wide(const char *utf8, wchar_t *wide, size_t wide_count)
{
    size_t converted;

    if (utf8 == NULL || wide == NULL || wide_count == 0) {
        return 0;
    }
    converted = mbstowcs(wide, utf8, wide_count);
    if (converted == (size_t)-1 || converted >= wide_count) {
        return 0;
    }
    wide[converted] = L'\0';
    return 1;
}

int wide_to_utf8(const wchar_t *wide, char *utf8, size_t utf8_size)
{
    size_t converted;

    if (wide == NULL || utf8 == NULL || utf8_size == 0) {
        return 0;
    }
    converted = wcstombs(utf8, wide, utf8_size);
    if (converted == (size_t)-1 || converted >= utf8_size) {
        return 0;
    }
    utf8[converted] = '\0';
    return 1;
}
#endif
