#ifndef STORAGE_H
#define STORAGE_H

#include <stddef.h>

#include "vehicle.h"

#define STORAGE_DEFAULT_PATH "vehicles.dat"

/* Devuelve 0 si todo fue bien, el numero de lineas omitidas si hubo avisos,
   o -1 si el archivo no se pudo leer o su cabecera es invalida. */
int storage_load(const char *path, VehicleStore *store, char *error, size_t error_size);

/* Devuelve 0 si guardo correctamente y -1 si hubo un error. */
int storage_save(const char *path, const VehicleStore *store, char *error, size_t error_size);

#endif
