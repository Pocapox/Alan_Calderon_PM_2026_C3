#ifndef JSON_STORE_H
#define JSON_STORE_H

#include "models.h"

int json_guardar(const SistemaPrestamos *sys, const char *ruta, char *errBuf, int errSize);
int json_cargar(SistemaPrestamos *sys, const char *ruta, char *errBuf, int errSize);

#endif /* JSON_STORE_H */
