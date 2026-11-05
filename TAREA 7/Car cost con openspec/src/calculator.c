#include "calculator.h"

#include <math.h>
#include <stdio.h>

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0) {
        snprintf(error, error_size, "%s", message);
    }
}

double vehicle_depreciation_per_year(const Vehicle *vehicle)
{
    return (vehicle->initial_value - vehicle->final_value) / vehicle->years_of_use;
}

double vehicle_cleaning_per_year(const Vehicle *vehicle)
{
    return vehicle->wash_cost * vehicle->washes_per_year;
}

double vehicle_fixed_per_year(const Vehicle *vehicle)
{
    return vehicle->insurance_per_year
         + vehicle->taxes_per_year
         + vehicle_depreciation_per_year(vehicle)
         + vehicle_cleaning_per_year(vehicle);
}

double vehicle_fixed_per_km(const Vehicle *vehicle)
{
    return vehicle_fixed_per_year(vehicle) / vehicle->km_per_year;
}

double trip_fuel_cost(const Vehicle *vehicle, double distance, double city_percent,
                      double highway_percent, double fuel_price_per_liter)
{
    double city_km = distance * city_percent / 100.0;
    double highway_km = distance * highway_percent / 100.0;

    return city_km / vehicle->city_km_per_l * fuel_price_per_liter
         + highway_km / vehicle->highway_km_per_l * fuel_price_per_liter;
}

CalcStatus calculator_compute(const Vehicle *vehicle, double distance, double city_percent,
                              double highway_percent, double fuel_price_per_liter,
                              TripCost *result, char *error, size_t error_size)
{
    double factor;

    if (vehicle == NULL || result == NULL) {
        set_error(error, error_size, "Vehiculo invalido.");
        return CALC_ERR_VEHICLE;
    }
    if (!vehicle_validate(vehicle, error, error_size)) {
        return CALC_ERR_VEHICLE;
    }
    if (!isfinite(distance) || distance <= 0.0) {
        set_error(error, error_size, "La distancia debe ser mayor que cero.");
        return CALC_ERR_DISTANCE;
    }
    if (!isfinite(city_percent) || !isfinite(highway_percent)
        || city_percent < 0.0 || city_percent > 100.0
        || highway_percent < 0.0 || highway_percent > 100.0
        || fabs((city_percent + highway_percent) - 100.0) > 1e-6) {
        set_error(error, error_size, "Los porcentajes deben sumar 100 y estar entre 0 y 100.");
        return CALC_ERR_PERCENTAGES;
    }
    if (!isfinite(fuel_price_per_liter) || fuel_price_per_liter <= 0.0) {
        set_error(error, error_size, "El precio del combustible debe ser mayor que cero.");
        return CALC_ERR_FUEL_PRICE;
    }

    result->depreciation_per_year = vehicle_depreciation_per_year(vehicle);
    result->cleaning_per_year = vehicle_cleaning_per_year(vehicle);
    result->fixed_per_year = vehicle_fixed_per_year(vehicle);
    result->fixed_per_km = vehicle_fixed_per_km(vehicle);
    result->city_km = distance * city_percent / 100.0;
    result->highway_km = distance * highway_percent / 100.0;
    result->fuel_cost = trip_fuel_cost(vehicle, distance, city_percent, highway_percent,
                                       fuel_price_per_liter);
    result->tires_cost = vehicle->tires_per_km * distance;
    result->maintenance_cost = vehicle->maintenance_per_km * distance;

    factor = distance / vehicle->km_per_year;
    result->insurance_cost = vehicle->insurance_per_year * factor;
    result->taxes_cost = vehicle->taxes_per_year * factor;
    result->depreciation_cost = result->depreciation_per_year * factor;
    result->cleaning_cost = result->cleaning_per_year * factor;

    result->total_cost = result->fuel_cost
                       + result->tires_cost
                       + result->maintenance_cost
                       + result->insurance_cost
                       + result->taxes_cost
                       + result->depreciation_cost
                       + result->cleaning_cost;
    result->cost_per_km = result->total_cost / distance;
    return CALC_OK;
}
