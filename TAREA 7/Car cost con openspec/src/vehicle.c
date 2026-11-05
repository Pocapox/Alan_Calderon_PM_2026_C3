#include "vehicle.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0) {
        snprintf(error, error_size, "%s", message);
    }
}

static int is_blank(const char *text)
{
    while (*text != '\0') {
        if (!isspace((unsigned char)*text)) {
            return 0;
        }
        text++;
    }
    return 1;
}

static char to_lower_ascii(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return (char)(c - 'A' + 'a');
    }
    return c;
}

void vehicle_store_init(VehicleStore *store)
{
    if (store != NULL) {
        store->count = 0;
        memset(store->items, 0, sizeof(store->items));
    }
}

void vehicle_copy(Vehicle *dest, const Vehicle *src)
{
    if (dest != NULL && src != NULL) {
        *dest = *src;
    }
}

int vehicle_name_equals_ci(const char *a, const char *b)
{
    if (a == NULL || b == NULL) {
        return 0;
    }
    while (*a != '\0' && *b != '\0') {
        if (to_lower_ascii(*a) != to_lower_ascii(*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

int vehicle_name_exists(const VehicleStore *store, const char *name, int exclude_index)
{
    int i;

    if (store == NULL || name == NULL) {
        return 0;
    }
    for (i = 0; i < store->count; i++) {
        if (i == exclude_index) {
            continue;
        }
        if (vehicle_name_equals_ci(store->items[i].name, name)) {
            return 1;
        }
    }
    return 0;
}

int vehicle_name_is_valid(const char *name, char *error, size_t error_size)
{
    size_t length;

    if (name == NULL || is_blank(name)) {
        set_error(error, error_size, "El nombre no puede estar vacio.");
        return 0;
    }
    length = strlen(name);
    if (length >= VEHICLE_NAME_MAX) {
        set_error(error, error_size, "El nombre es demasiado largo (maximo 127 caracteres).");
        return 0;
    }
    if (strchr(name, '|') != NULL || strchr(name, '\n') != NULL || strchr(name, '\r') != NULL) {
        set_error(error, error_size, "El nombre no puede contener '|' ni saltos de linea.");
        return 0;
    }
    return 1;
}

int vehicle_validate(const Vehicle *vehicle, char *error, size_t error_size)
{
    if (vehicle == NULL) {
        set_error(error, error_size, "Vehiculo invalido.");
        return 0;
    }
    if (!vehicle_name_is_valid(vehicle->name, error, error_size)) {
        return 0;
    }
    if (!isfinite(vehicle->city_km_per_l) || vehicle->city_km_per_l <= 0.0) {
        set_error(error, error_size, "El consumo urbano debe ser mayor que cero.");
        return 0;
    }
    if (!isfinite(vehicle->highway_km_per_l) || vehicle->highway_km_per_l <= 0.0) {
        set_error(error, error_size, "El consumo de carretera debe ser mayor que cero.");
        return 0;
    }
    if (!isfinite(vehicle->km_per_year) || vehicle->km_per_year <= 0.0) {
        set_error(error, error_size, "El kilometraje promedio anual debe ser mayor que cero.");
        return 0;
    }
    if (!isfinite(vehicle->years_of_use) || vehicle->years_of_use <= 0.0) {
        set_error(error, error_size, "Los anios de uso deben ser mayores que cero.");
        return 0;
    }
    if (!isfinite(vehicle->insurance_per_year) || vehicle->insurance_per_year < 0.0) {
        set_error(error, error_size, "El seguro anual no puede ser negativo.");
        return 0;
    }
    if (!isfinite(vehicle->taxes_per_year) || vehicle->taxes_per_year < 0.0) {
        set_error(error, error_size, "Los impuestos anuales no pueden ser negativos.");
        return 0;
    }
    if (!isfinite(vehicle->tires_per_km) || vehicle->tires_per_km < 0.0) {
        set_error(error, error_size, "El costo de neumaticos por km no puede ser negativo.");
        return 0;
    }
    if (!isfinite(vehicle->maintenance_per_km) || vehicle->maintenance_per_km < 0.0) {
        set_error(error, error_size, "El costo de mantenimiento por km no puede ser negativo.");
        return 0;
    }
    if (!isfinite(vehicle->wash_cost) || vehicle->wash_cost < 0.0) {
        set_error(error, error_size, "El costo por lavado no puede ser negativo.");
        return 0;
    }
    if (!isfinite(vehicle->washes_per_year) || vehicle->washes_per_year < 0.0) {
        set_error(error, error_size, "La cantidad de lavados por ano no puede ser negativa.");
        return 0;
    }
    if (!isfinite(vehicle->initial_value) || vehicle->initial_value < 0.0) {
        set_error(error, error_size, "El valor inicial no puede ser negativo.");
        return 0;
    }
    if (!isfinite(vehicle->final_value) || vehicle->final_value < 0.0) {
        set_error(error, error_size, "El valor final no puede ser negativo.");
        return 0;
    }
    if (vehicle->final_value > vehicle->initial_value) {
        set_error(error, error_size, "El valor final no puede superar el valor inicial (depreciacion negativa).");
        return 0;
    }
    return 1;
}

int vehicle_store_add(VehicleStore *store, const Vehicle *vehicle, char *error, size_t error_size)
{
    if (store == NULL || vehicle == NULL) {
        set_error(error, error_size, "Vehiculo invalido.");
        return 0;
    }
    if (store->count >= MAX_VEHICLES) {
        set_error(error, error_size, "Se alcanzo el limite de vehiculos registrados.");
        return 0;
    }
    if (!vehicle_validate(vehicle, error, error_size)) {
        return 0;
    }
    if (vehicle_name_exists(store, vehicle->name, -1)) {
        set_error(error, error_size, "Ya existe un vehiculo con ese nombre.");
        return 0;
    }
    store->items[store->count] = *vehicle;
    store->count++;
    return 1;
}

int vehicle_store_update(VehicleStore *store, int index, const Vehicle *vehicle, char *error, size_t error_size)
{
    if (store == NULL || vehicle == NULL || index < 0 || index >= store->count) {
        set_error(error, error_size, "Vehiculo inexistente.");
        return 0;
    }
    if (!vehicle_validate(vehicle, error, error_size)) {
        return 0;
    }
    if (vehicle_name_exists(store, vehicle->name, index)) {
        set_error(error, error_size, "Ya existe otro vehiculo con ese nombre.");
        return 0;
    }
    store->items[index] = *vehicle;
    return 1;
}

int vehicle_store_remove(VehicleStore *store, int index)
{
    int i;

    if (store == NULL || index < 0 || index >= store->count) {
        return 0;
    }
    for (i = index; i < store->count - 1; i++) {
        store->items[i] = store->items[i + 1];
    }
    store->count--;
    memset(&store->items[store->count], 0, sizeof(store->items[store->count]));
    return 1;
}
