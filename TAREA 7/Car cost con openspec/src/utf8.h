#ifndef UTF8_H
#define UTF8_H

#include <stddef.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <wchar.h>
#endif

/*
 * Convierte una cadena UTF-8 a UTF-16 en 'wide'.
 * 'wide_count' es la capacidad en wchar_t del buffer de destino.
 * Devuelve 1 en exito y 0 si falla (parametros invalidos, buffer insuficiente o error de conversion).
 */
int utf8_to_wide(const char *utf8, wchar_t *wide, size_t wide_count);

/*
 * Convierte una cadena UTF-16 a UTF-8 en 'utf8'.
 * 'utf8_size' es la capacidad en bytes del buffer de destino.
 * Devuelve 1 en exito y 0 si falla (parametros invalidos, buffer insuficiente o error de conversion).
 */
int wide_to_utf8(const wchar_t *wide, char *utf8, size_t utf8_size);

#endif
