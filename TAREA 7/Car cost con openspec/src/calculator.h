#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stddef.h>

#include "vehicle.h"

typedef enum {
    CALC_OK = 0,
    CALC_ERR_VEHICLE,
    CALC_ERR_DISTANCE,
    CALC_ERR_PERCENTAGES,
    CALC_ERR_FUEL_PRICE
} CalcStatus;

typedef struct {
    double depreciation_per_year;
    double cleaning_per_year;
    double fixed_per_year;
    double fixed_per_km;
    double city_km;
    double highway_km;
    double fuel_cost;
    double tires_cost;
    double maintenance_cost;
    double insurance_cost;
    double taxes_cost;
    double depreciation_cost;
    double cleaning_cost;
    double total_cost;
    double cost_per_km;
} TripCost;

double vehicle_depreciation_per_year(const Vehicle *vehicle);
double vehicle_cleaning_per_year(const Vehicle *vehicle);
double vehicle_fixed_per_year(const Vehicle *vehicle);
double vehicle_fixed_per_km(const Vehicle *vehicle);

double trip_fuel_cost(const Vehicle *vehicle, double distance, double city_percent,
                      double highway_percent, double fuel_price_per_liter);

CalcStatus calculator_compute(const Vehicle *vehicle, double distance, double city_percent,
                              double highway_percent, double fuel_price_per_liter,
                              TripCost *result, char *error, size_t error_size);

#endif
