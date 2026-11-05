#ifndef VEHICLE_H
#define VEHICLE_H

#include <stddef.h>

#define VEHICLE_NAME_MAX 128
#define MAX_VEHICLES 100

typedef struct {
    char name[VEHICLE_NAME_MAX];
    double city_km_per_l;
    double highway_km_per_l;
    double insurance_per_year;
    double tires_per_km;
    double maintenance_per_km;
    double initial_value;
    double years_of_use;
    double final_value;
    double taxes_per_year;
    double wash_cost;
    double washes_per_year;
    double km_per_year;
} Vehicle;

typedef struct {
    Vehicle items[MAX_VEHICLES];
    int count;
} VehicleStore;

void vehicle_store_init(VehicleStore *store);
void vehicle_copy(Vehicle *dest, const Vehicle *src);
int vehicle_name_equals_ci(const char *a, const char *b);
int vehicle_name_exists(const VehicleStore *store, const char *name, int exclude_index);
int vehicle_name_is_valid(const char *name, char *error, size_t error_size);
int vehicle_validate(const Vehicle *vehicle, char *error, size_t error_size);
int vehicle_store_add(VehicleStore *store, const Vehicle *vehicle, char *error, size_t error_size);
int vehicle_store_update(VehicleStore *store, int index, const Vehicle *vehicle, char *error, size_t error_size);
int vehicle_store_remove(VehicleStore *store, int index);

#endif
