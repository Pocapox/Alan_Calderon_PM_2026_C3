#include <math.h>
#include <stdio.h>
#include <string.h>

#include "calculator.h"
#include "input.h"
#include "parse.h"
#include "storage.h"
#include "utf8.h"
#include "vehicle.h"

static int failures = 0;
static int checks = 0;

#define CHECK(condition)                                                          \
    do {                                                                          \
        checks++;                                                                 \
        if (!(condition)) {                                                       \
            printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #condition);          \
            failures++;                                                           \
        }                                                                         \
    } while (0)

#define CHECK_NEAR(actual, expected, tolerance)                                   \
    do {                                                                          \
        double actual_value = (actual);                                           \
        double expected_value = (expected);                                       \
        checks++;                                                                 \
        if (fabs(actual_value - expected_value) > (tolerance)) {                  \
            printf("FALLO %s:%d: %.12f != %.12f\n", __FILE__, __LINE__,           \
                   actual_value, expected_value);                                 \
            failures++;                                                           \
        }                                                                         \
    } while (0)

static Vehicle make_vehicle(void)
{
    Vehicle vehicle;

    memset(&vehicle, 0, sizeof(vehicle));
    snprintf(vehicle.name, sizeof(vehicle.name), "Auto");
    vehicle.city_km_per_l = 10.0;
    vehicle.highway_km_per_l = 20.0;
    vehicle.insurance_per_year = 30000.0;
    vehicle.tires_per_km = 2.0;
    vehicle.maintenance_per_km = 3.0;
    vehicle.initial_value = 1000000.0;
    vehicle.years_of_use = 5.0;
    vehicle.final_value = 400000.0;
    vehicle.taxes_per_year = 10000.0;
    vehicle.wash_cost = 500.0;
    vehicle.washes_per_year = 12.0;
    vehicle.km_per_year = 12000.0;
    return vehicle;
}

static void test_vehicle_validation(void)
{
    Vehicle vehicle;
    VehicleStore store;
    char error[256];

    vehicle = make_vehicle();
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 1);

    vehicle = make_vehicle();
    vehicle.city_km_per_l = 0.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.highway_km_per_l = 0.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.km_per_year = 0.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.years_of_use = 0.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.insurance_per_year = -1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.tires_per_km = -1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.maintenance_per_km = -1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.taxes_per_year = -1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.wash_cost = -1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.washes_per_year = -1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.initial_value = -1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.final_value = -1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    vehicle.final_value = vehicle.initial_value + 1.0;
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    snprintf(vehicle.name, sizeof(vehicle.name), "   ");
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    vehicle = make_vehicle();
    snprintf(vehicle.name, sizeof(vehicle.name), "Auto|Raro");
    CHECK(vehicle_validate(&vehicle, error, sizeof(error)) == 0);

    {
        char valid_127[128];
        memset(valid_127, 'a', 127);
        valid_127[127] = '\0';
        CHECK(vehicle_name_is_valid(valid_127, error, sizeof(error)) == 1);

        char too_long[256];
        memset(too_long, 'a', 128);
        too_long[128] = '\0';
        CHECK(vehicle_name_is_valid(too_long, error, sizeof(error)) == 0);

        CHECK(vehicle_name_is_valid("Camión de José", error, sizeof(error)) == 1);
    }

    vehicle_store_init(&store);
    vehicle = make_vehicle();
    CHECK(vehicle_store_add(&store, &vehicle, error, sizeof(error)) == 1);
    vehicle = make_vehicle();
    snprintf(vehicle.name, sizeof(vehicle.name), "auto");
    CHECK(vehicle_store_add(&store, &vehicle, error, sizeof(error)) == 0);
    CHECK(store.count == 1);
}

static void test_calculator_helpers(void)
{
    Vehicle vehicle = make_vehicle();

    CHECK_NEAR(vehicle_depreciation_per_year(&vehicle), 120000.0, 1e-9);
    CHECK_NEAR(vehicle_cleaning_per_year(&vehicle), 6000.0, 1e-9);
    CHECK_NEAR(vehicle_fixed_per_year(&vehicle), 166000.0, 1e-9);
    CHECK_NEAR(vehicle_fixed_per_km(&vehicle), 166000.0 / 12000.0, 1e-9);
}

static void test_fuel_cost(void)
{
    Vehicle vehicle = make_vehicle();

    CHECK_NEAR(trip_fuel_cost(&vehicle, 100.0, 60.0, 40.0, 100.0), 800.0, 1e-9);
    CHECK_NEAR(trip_fuel_cost(&vehicle, 100.0, 100.0, 0.0, 100.0), 1000.0, 1e-9);
    CHECK_NEAR(trip_fuel_cost(&vehicle, 100.0, 0.0, 100.0, 100.0), 500.0, 1e-9);
}

static void test_full_calculation(void)
{
    Vehicle vehicle = make_vehicle();
    TripCost result;
    char error[256];

    CHECK(calculator_compute(&vehicle, 100.0, 60.0, 40.0, 100.0, &result,
                             error, sizeof(error)) == CALC_OK);
    CHECK_NEAR(result.city_km, 60.0, 1e-9);
    CHECK_NEAR(result.highway_km, 40.0, 1e-9);
    CHECK_NEAR(result.fuel_cost, 800.0, 1e-9);
    CHECK_NEAR(result.tires_cost, 200.0, 1e-9);
    CHECK_NEAR(result.maintenance_cost, 300.0, 1e-9);
    CHECK_NEAR(result.insurance_cost, 250.0, 1e-9);
    CHECK_NEAR(result.taxes_cost, 10000.0 / 120.0, 1e-9);
    CHECK_NEAR(result.depreciation_cost, 1000.0, 1e-9);
    CHECK_NEAR(result.cleaning_cost, 50.0, 1e-9);
    CHECK_NEAR(result.total_cost, 2683.333333333333, 1e-9);
    CHECK_NEAR(result.cost_per_km, 26.833333333333, 1e-9);
}

static void test_edge_cases(void)
{
    Vehicle vehicle = make_vehicle();
    TripCost result;
    char error[256];

    CHECK(calculator_compute(&vehicle, 0.0, 50.0, 50.0, 100.0, &result,
                             error, sizeof(error)) == CALC_ERR_DISTANCE);
    CHECK(calculator_compute(&vehicle, -10.0, 50.0, 50.0, 100.0, &result,
                             error, sizeof(error)) == CALC_ERR_DISTANCE);
    CHECK(calculator_compute(&vehicle, 100.0, 50.0, 40.0, 100.0, &result,
                             error, sizeof(error)) == CALC_ERR_PERCENTAGES);
    CHECK(calculator_compute(&vehicle, 100.0, -10.0, 110.0, 100.0, &result,
                             error, sizeof(error)) == CALC_ERR_PERCENTAGES);
    CHECK(calculator_compute(&vehicle, 100.0, 50.0, 50.0, 0.0, &result,
                             error, sizeof(error)) == CALC_ERR_FUEL_PRICE);

    vehicle.km_per_year = 0.0;
    CHECK(calculator_compute(&vehicle, 100.0, 50.0, 50.0, 100.0, &result,
                             error, sizeof(error)) == CALC_ERR_VEHICLE);

    vehicle = make_vehicle();
    CHECK(calculator_compute(&vehicle, 100.0, 100.0, 0.0, 100.0, &result,
                             error, sizeof(error)) == CALC_OK);
    CHECK_NEAR(result.fuel_cost, 1000.0, 1e-9);
    CHECK(calculator_compute(&vehicle, 100.0, 0.0, 100.0, 100.0, &result,
                             error, sizeof(error)) == CALC_OK);
    CHECK_NEAR(result.fuel_cost, 500.0, 1e-9);
}

static void test_storage(void)
{
    const char *path = "test_vehicles.dat";
    VehicleStore store;
    VehicleStore loaded;
    Vehicle first;
    Vehicle second;
    Vehicle third;
    char error[256];
    FILE *file;

    remove(path);
    remove("test_vehicles.dat.tmp");

    CHECK(storage_load(path, &store, error, sizeof(error)) == 0);
    CHECK(store.count == 0);

    vehicle_store_init(&store);
    first = make_vehicle();
    second = make_vehicle();
    snprintf(second.name, sizeof(second.name), "Camioneta");
    second.city_km_per_l = 8.0;
    second.highway_km_per_l = 12.0;
    third = make_vehicle();
    snprintf(third.name, sizeof(third.name), "Camión de José");
    CHECK(vehicle_store_add(&store, &first, error, sizeof(error)) == 1);
    CHECK(vehicle_store_add(&store, &second, error, sizeof(error)) == 1);
    CHECK(vehicle_store_add(&store, &third, error, sizeof(error)) == 1);

    CHECK(storage_save(path, &store, error, sizeof(error)) == 0);
    file = fopen("test_vehicles.dat.tmp", "r");
    CHECK(file == NULL);
    if (file != NULL) {
        fclose(file);
    }

    CHECK(storage_load(path, &loaded, error, sizeof(error)) == 0);
    CHECK(loaded.count == 3);
    CHECK(strcmp(loaded.items[0].name, "Auto") == 0);
    CHECK(strcmp(loaded.items[1].name, "Camioneta") == 0);
    CHECK(strcmp(loaded.items[2].name, "Camión de José") == 0);
    CHECK_NEAR(loaded.items[1].city_km_per_l, 8.0, 1e-9);

    file = fopen(path, "a");
    CHECK(file != NULL);
    if (file != NULL) {
        fprintf(file, "linea|invalida\n");
        fclose(file);
    }
    CHECK(storage_load(path, &loaded, error, sizeof(error)) == 1);
    CHECK(loaded.count == 3);

    file = fopen(path, "w");
    CHECK(file != NULL);
    if (file != NULL) {
        fprintf(file, "cabecera incorrecta\n");
        fclose(file);
    }
    CHECK(storage_load(path, &loaded, error, sizeof(error)) == -1);
    CHECK(loaded.count == 0);

    remove(path);
    remove("test_vehicles.dat.tmp");
}

static void test_input_parsing(void)
{
    double real_value;
    int integer_value;
    int boolean_value;
    char text[32];

    CHECK(input_parse_double("12.5", &real_value) == 1);
    CHECK_NEAR(real_value, 12.5, 1e-9);
    CHECK(input_parse_double("12,5", &real_value) == 1);
    CHECK_NEAR(real_value, 12.5, 1e-9);
    CHECK(input_parse_double("  -3  ", &real_value) == 1);
    CHECK_NEAR(real_value, -3.0, 1e-9);
    CHECK(input_parse_double("1e3", &real_value) == 1);
    CHECK_NEAR(real_value, 1000.0, 1e-9);
    CHECK(input_parse_double("abc", &real_value) == 0);
    CHECK(input_parse_double("12abc", &real_value) == 0);
    CHECK(input_parse_double("", &real_value) == 0);

    CHECK(input_parse_int("5", &integer_value) == 1);
    CHECK(integer_value == 5);
    CHECK(input_parse_int("5x", &integer_value) == 0);
    CHECK(input_parse_int("3.5", &integer_value) == 0);
    CHECK(input_parse_int("", &integer_value) == 0);

    CHECK(input_parse_yes_no("s", &boolean_value) == 1 && boolean_value == 1);
    CHECK(input_parse_yes_no("SI", &boolean_value) == 1 && boolean_value == 1);
    CHECK(input_parse_yes_no("n", &boolean_value) == 1 && boolean_value == 0);
    CHECK(input_parse_yes_no("NO", &boolean_value) == 1 && boolean_value == 0);
    CHECK(input_parse_yes_no("quizas", &boolean_value) == 0);

    snprintf(text, sizeof(text), "  hola mundo  ");
    input_trim(text);
    CHECK(strcmp(text, "hola mundo") == 0);
}

static void test_parse(void)
{
    double val_d;
    int val_i;
    char text[64];

    CHECK(parse_double("123.45", &val_d) == 1);
    CHECK_NEAR(val_d, 123.45, 1e-9);
    CHECK(parse_double("  123,45  ", &val_d) == 1);
    CHECK_NEAR(val_d, 123.45, 1e-9);
    CHECK(parse_double("invalid", &val_d) == 0);
    CHECK(parse_double("", &val_d) == 0);
    CHECK(parse_double(NULL, &val_d) == 0);

    CHECK(parse_int("42", &val_i) == 1);
    CHECK(val_i == 42);
    CHECK(parse_int("  -99  ", &val_i) == 1);
    CHECK(val_i == -99);
    CHECK(parse_int("12.3", &val_i) == 0);
    CHECK(parse_int("abc", &val_i) == 0);
    CHECK(parse_int(NULL, &val_i) == 0);

    snprintf(text, sizeof(text), "   espacios   ");
    parse_trim(text);
    CHECK(strcmp(text, "espacios") == 0);
}

static void test_utf8(void)
{
    wchar_t wide[128];
    char utf8_out[128];
    const char *original = "Camión de José";
    wchar_t small_wide[4];
    char small_utf8[4];

    /* Conversion ida y vuelta con acentos */
    CHECK(utf8_to_wide(original, wide, sizeof(wide) / sizeof(wide[0])) == 1);
    CHECK(wide_to_utf8(wide, utf8_out, sizeof(utf8_out)) == 1);
    CHECK(strcmp(original, utf8_out) == 0);

    /* Buffer insuficiente */
    CHECK(utf8_to_wide(original, small_wide, sizeof(small_wide) / sizeof(small_wide[0])) == 0);
    CHECK(wide_to_utf8(wide, small_utf8, sizeof(small_utf8)) == 0);

    /* Cadenas vacias */
    CHECK(utf8_to_wide("", wide, sizeof(wide) / sizeof(wide[0])) == 1);
    CHECK(wide[0] == L'\0');
    CHECK(wide_to_utf8(L"", utf8_out, sizeof(utf8_out)) == 1);
    CHECK(utf8_out[0] == '\0');

    /* Parametros nulos o tamanos cero */
    CHECK(utf8_to_wide(NULL, wide, 10) == 0);
    CHECK(utf8_to_wide("test", NULL, 10) == 0);
    CHECK(utf8_to_wide("test", wide, 0) == 0);
    CHECK(wide_to_utf8(NULL, utf8_out, 10) == 0);
    CHECK(wide_to_utf8(L"test", NULL, 10) == 0);
    CHECK(wide_to_utf8(L"test", utf8_out, 0) == 0);
}

int main(void)
{
    test_vehicle_validation();
    test_calculator_helpers();
    test_fuel_cost();
    test_full_calculation();
    test_edge_cases();
    test_storage();
    test_input_parsing();
    test_parse();
    test_utf8();

    printf("\n%d comprobaciones, %d fallos\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
