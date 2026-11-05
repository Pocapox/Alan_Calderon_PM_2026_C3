#include "storage.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define STORAGE_HEADER "carcost 1"
#define STORAGE_LINE_MAX 4096
#define STORAGE_FIELD_COUNT 13

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0) {
        snprintf(error, error_size, "%s", message);
    }
}

static void trim_line_end(char *text)
{
    size_t length = strlen(text);

    while (length > 0) {
        char last = text[length - 1];
        if (last != '\n' && last != '\r') {
            break;
        }
        text[length - 1] = '\0';
        length--;
    }
}

static int parse_double_field(const char *token, double *value)
{
    char *end = NULL;
    double parsed;

    if (token == NULL || *token == '\0') {
        return 0;
    }
    errno = 0;
    parsed = strtod(token, &end);
    if (end == token || *end != '\0' || errno == ERANGE || !isfinite(parsed)) {
        return 0;
    }
    *value = parsed;
    return 1;
}

int storage_load(const char *path, VehicleStore *store, char *error, size_t error_size)
{
    FILE *file;
    char line[STORAGE_LINE_MAX];
    int skipped = 0;

    if (path == NULL || store == NULL) {
        set_error(error, error_size, "Ruta o catalogo invalido.");
        return -1;
    }

    vehicle_store_init(store);
    file = fopen(path, "r");
    if (file == NULL) {
        if (errno == ENOENT) {
            return 0;
        }
        set_error(error, error_size, "No se pudo abrir el archivo de vehiculos.");
        return -1;
    }

    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        set_error(error, error_size, "El archivo de vehiculos esta vacio o no tiene cabecera.");
        return -1;
    }
    trim_line_end(line);
    if (strcmp(line, STORAGE_HEADER) != 0) {
        fclose(file);
        set_error(error, error_size, "Cabecera de archivo de vehiculos invalida.");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *fields[STORAGE_FIELD_COUNT + 1];
        char *token;
        int field_count = 0;
        int index;
        Vehicle vehicle;
        double *targets[STORAGE_FIELD_COUNT - 1];
        int valid = 1;

        trim_line_end(line);
        if (line[0] == '\0') {
            continue;
        }

        token = strtok(line, "|");
        while (token != NULL && field_count <= STORAGE_FIELD_COUNT) {
            fields[field_count++] = token;
            token = strtok(NULL, "|");
        }
        if (field_count != STORAGE_FIELD_COUNT) {
            skipped++;
            continue;
        }

        memset(&vehicle, 0, sizeof(vehicle));
        snprintf(vehicle.name, sizeof(vehicle.name), "%s", fields[0]);
        targets[0] = &vehicle.city_km_per_l;
        targets[1] = &vehicle.highway_km_per_l;
        targets[2] = &vehicle.insurance_per_year;
        targets[3] = &vehicle.tires_per_km;
        targets[4] = &vehicle.maintenance_per_km;
        targets[5] = &vehicle.initial_value;
        targets[6] = &vehicle.years_of_use;
        targets[7] = &vehicle.final_value;
        targets[8] = &vehicle.taxes_per_year;
        targets[9] = &vehicle.wash_cost;
        targets[10] = &vehicle.washes_per_year;
        targets[11] = &vehicle.km_per_year;

        for (index = 0; index < STORAGE_FIELD_COUNT - 1; index++) {
            if (!parse_double_field(fields[index + 1], targets[index])) {
                valid = 0;
                break;
            }
        }
        if (!valid || !vehicle_validate(&vehicle, NULL, 0) || store->count >= MAX_VEHICLES) {
            skipped++;
            continue;
        }
        store->items[store->count] = vehicle;
        store->count++;
    }

    if (ferror(file)) {
        fclose(file);
        set_error(error, error_size, "Error al leer el archivo de vehiculos.");
        return -1;
    }
    fclose(file);
    if (skipped > 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Se omitieron %d lineas invalidas del archivo de vehiculos.",
                     skipped);
        }
    }
    return skipped;
}

int storage_save(const char *path, const VehicleStore *store, char *error, size_t error_size)
{
    char temporary[STORAGE_LINE_MAX];
    FILE *file;
    int index;
    int written;

    if (path == NULL || store == NULL) {
        set_error(error, error_size, "Ruta o catalogo invalido.");
        return -1;
    }
    written = snprintf(temporary, sizeof(temporary), "%s.tmp", path);
    if (written < 0 || (size_t)written >= sizeof(temporary)) {
        set_error(error, error_size, "La ruta del archivo de vehiculos es demasiado larga.");
        return -1;
    }

    file = fopen(temporary, "w");
    if (file == NULL) {
        set_error(error, error_size, "No se pudo crear el archivo temporal de vehiculos.");
        return -1;
    }
    if (fprintf(file, "%s\n", STORAGE_HEADER) < 0) {
        fclose(file);
        remove(temporary);
        set_error(error, error_size, "No se pudo escribir la cabecera del archivo de vehiculos.");
        return -1;
    }
    for (index = 0; index < store->count; index++) {
        const Vehicle *vehicle = &store->items[index];
        if (fprintf(file, "%s|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g\n",
                    vehicle->name,
                    vehicle->city_km_per_l,
                    vehicle->highway_km_per_l,
                    vehicle->insurance_per_year,
                    vehicle->tires_per_km,
                    vehicle->maintenance_per_km,
                    vehicle->initial_value,
                    vehicle->years_of_use,
                    vehicle->final_value,
                    vehicle->taxes_per_year,
                    vehicle->wash_cost,
                    vehicle->washes_per_year,
                    vehicle->km_per_year) < 0) {
            fclose(file);
            remove(temporary);
            set_error(error, error_size, "No se pudo escribir un vehiculo en el archivo.");
            return -1;
        }
    }
    if (fclose(file) != 0) {
        remove(temporary);
        set_error(error, error_size, "No se pudo cerrar el archivo temporal de vehiculos.");
        return -1;
    }

#ifdef _WIN32
    if (!MoveFileExA(temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(temporary);
        set_error(error, error_size, "No se pudo reemplazar el archivo de vehiculos.");
        return -1;
    }
#else
    if (rename(temporary, path) != 0) {
        remove(temporary);
        set_error(error, error_size, "No se pudo reemplazar el archivo de vehiculos.");
        return -1;
    }
#endif
    return 0;
}
