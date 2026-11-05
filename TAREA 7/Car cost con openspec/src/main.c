#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "calculator.h"
#include "input.h"
#include "storage.h"
#include "vehicle.h"

static void print_menu(void)
{
    printf("\n=== Car Cost ===\n");
    printf("1. Crear vehiculo\n");
    printf("2. Listar vehiculos\n");
    printf("3. Editar vehiculo\n");
    printf("4. Eliminar vehiculo\n");
    printf("5. Calcular viaje\n");
    printf("0. Salir\n");
}

static void print_vehicle(const Vehicle *vehicle, int number)
{
    printf("%d) %s\n", number, vehicle->name);
    printf("   Consumo: %.2f km/L ciudad | %.2f km/L carretera\n",
           vehicle->city_km_per_l, vehicle->highway_km_per_l);
    printf("   Seguro: %.2f DOP/anio | Impuestos: %.2f DOP/anio\n",
           vehicle->insurance_per_year, vehicle->taxes_per_year);
    printf("   Neumaticos: %.2f DOP/km | Mantenimiento: %.2f DOP/km\n",
           vehicle->tires_per_km, vehicle->maintenance_per_km);
    printf("   Valor inicial: %.2f | Anios de uso: %.2f | Valor final: %.2f\n",
           vehicle->initial_value, vehicle->years_of_use, vehicle->final_value);
    printf("   Limpieza: %.2f DOP/lavado x %.2f lavados/anio\n",
           vehicle->wash_cost, vehicle->washes_per_year);
    printf("   Kilometraje promedio anual: %.2f km\n", vehicle->km_per_year);
}

static void list_vehicles(const VehicleStore *store)
{
    int index;

    if (store->count == 0) {
        printf("No hay vehiculos registrados.\n");
        return;
    }
    for (index = 0; index < store->count; index++) {
        print_vehicle(&store->items[index], index + 1);
    }
}

static int choose_vehicle(VehicleStore *store, int *index)
{
    int choice;

    if (store->count == 0) {
        printf("No hay vehiculos registrados.\n");
        return 0;
    }
    list_vehicles(store);
    if (!input_read_int_range("Seleccione el numero del vehiculo: ", 1, store->count, &choice)) {
        return 0;
    }
    *index = choice - 1;
    return 1;
}

static int read_new_name(VehicleStore *store, char *name, size_t size, int exclude_index)
{
    char error[256];

    for (;;) {
        if (!input_read_text("Nombre: ", name, size)) {
            return 0;
        }
        if (!vehicle_name_is_valid(name, error, sizeof(error))) {
            printf("Error: %s\n", error);
            continue;
        }
        if (vehicle_name_exists(store, name, exclude_index)) {
            printf("Error: ya existe un vehiculo con ese nombre.\n");
            continue;
        }
        return 1;
    }
}

static void create_vehicle(VehicleStore *store)
{
    Vehicle vehicle;
    char error[256];

    memset(&vehicle, 0, sizeof(vehicle));
    if (!read_new_name(store, vehicle.name, sizeof(vehicle.name), -1)) {
        return;
    }
    if (!input_read_double("Consumo ciudad (km/L): ", &vehicle.city_km_per_l)) return;
    if (!input_read_double("Consumo carretera (km/L): ", &vehicle.highway_km_per_l)) return;
    if (!input_read_double("Seguro anual (DOP): ", &vehicle.insurance_per_year)) return;
    if (!input_read_double("Costo de neumaticos por km (DOP): ", &vehicle.tires_per_km)) return;
    if (!input_read_double("Mantenimiento por km (DOP): ", &vehicle.maintenance_per_km)) return;
    if (!input_read_double("Valor inicial (DOP): ", &vehicle.initial_value)) return;
    if (!input_read_double("Anios de uso: ", &vehicle.years_of_use)) return;
    if (!input_read_double("Valor final (DOP): ", &vehicle.final_value)) return;
    if (!input_read_double("Impuestos anuales (DOP): ", &vehicle.taxes_per_year)) return;
    if (!input_read_double("Costo por lavado (DOP): ", &vehicle.wash_cost)) return;
    if (!input_read_double("Lavados por anio: ", &vehicle.washes_per_year)) return;
    if (!input_read_double("Kilometraje promedio anual (km): ", &vehicle.km_per_year)) return;

    if (!vehicle_store_add(store, &vehicle, error, sizeof(error))) {
        printf("Error: %s\n", error);
        return;
    }
    if (storage_save(STORAGE_DEFAULT_PATH, store, error, sizeof(error)) != 0) {
        vehicle_store_remove(store, store->count - 1);
        printf("Error al guardar: %s\n", error);
        return;
    }
    printf("Vehiculo creado correctamente.\n");
}

static void edit_vehicle(VehicleStore *store)
{
    Vehicle updated;
    Vehicle original;
    char error[256];
    char name[VEHICLE_NAME_MAX];
    int index;
    int changed;

    if (!choose_vehicle(store, &index)) {
        return;
    }
    original = store->items[index];
    updated = original;

    printf("Deje vacio un campo para conservar su valor actual.\n");
    if (!input_read_text_optional("Nombre", original.name, name, sizeof(name))) return;
    if (strcmp(name, original.name) != 0) {
        if (!vehicle_name_is_valid(name, error, sizeof(error))) {
            printf("Error: %s\n", error);
            return;
        }
        if (vehicle_name_exists(store, name, index)) {
            printf("Error: ya existe otro vehiculo con ese nombre.\n");
            return;
        }
        snprintf(updated.name, sizeof(updated.name), "%s", name);
    }

    if (!input_read_double_optional("Consumo ciudad (km/L)", original.city_km_per_l,
                                    &updated.city_km_per_l, &changed)) return;
    if (!input_read_double_optional("Consumo carretera (km/L)", original.highway_km_per_l,
                                    &updated.highway_km_per_l, &changed)) return;
    if (!input_read_double_optional("Seguro anual (DOP)", original.insurance_per_year,
                                    &updated.insurance_per_year, &changed)) return;
    if (!input_read_double_optional("Neumaticos por km (DOP)", original.tires_per_km,
                                    &updated.tires_per_km, &changed)) return;
    if (!input_read_double_optional("Mantenimiento por km (DOP)", original.maintenance_per_km,
                                    &updated.maintenance_per_km, &changed)) return;
    if (!input_read_double_optional("Valor inicial (DOP)", original.initial_value,
                                    &updated.initial_value, &changed)) return;
    if (!input_read_double_optional("Anios de uso", original.years_of_use,
                                    &updated.years_of_use, &changed)) return;
    if (!input_read_double_optional("Valor final (DOP)", original.final_value,
                                    &updated.final_value, &changed)) return;
    if (!input_read_double_optional("Impuestos anuales (DOP)", original.taxes_per_year,
                                    &updated.taxes_per_year, &changed)) return;
    if (!input_read_double_optional("Costo por lavado (DOP)", original.wash_cost,
                                    &updated.wash_cost, &changed)) return;
    if (!input_read_double_optional("Lavados por anio", original.washes_per_year,
                                    &updated.washes_per_year, &changed)) return;
    if (!input_read_double_optional("Kilometraje promedio anual (km)", original.km_per_year,
                                    &updated.km_per_year, &changed)) return;

    if (!vehicle_store_update(store, index, &updated, error, sizeof(error))) {
        printf("Error: %s\n", error);
        return;
    }
    if (storage_save(STORAGE_DEFAULT_PATH, store, error, sizeof(error)) != 0) {
        store->items[index] = original;
        printf("Error al guardar: %s\n", error);
        return;
    }
    printf("Vehiculo actualizado correctamente.\n");
}

static void delete_vehicle(VehicleStore *store)
{
    Vehicle removed;
    char error[256];
    char prompt[128];
    int index;
    int confirmed;
    int i;

    if (!choose_vehicle(store, &index)) {
        return;
    }
    snprintf(prompt, sizeof(prompt), "Eliminar '%s'? (s/n): ", store->items[index].name);
    if (!input_read_yes_no(prompt, &confirmed)) {
        return;
    }
    if (!confirmed) {
        printf("Operacion cancelada.\n");
        return;
    }
    removed = store->items[index];
    if (!vehicle_store_remove(store, index)) {
        printf("Error: no se pudo eliminar el vehiculo.\n");
        return;
    }
    if (storage_save(STORAGE_DEFAULT_PATH, store, error, sizeof(error)) != 0) {
        for (i = store->count; i > index; i--) {
            store->items[i] = store->items[i - 1];
        }
        store->items[index] = removed;
        store->count++;
        printf("Error al guardar: %s\n", error);
        return;
    }
    printf("Vehiculo eliminado correctamente.\n");
}

static void calculate_trip(VehicleStore *store)
{
    char error[256];
    TripCost result;
    double distance;
    double city_percent;
    double highway_percent;
    double fuel_price;
    int index;

    if (!choose_vehicle(store, &index)) {
        return;
    }
    if (!input_read_double("Distancia total (km): ", &distance)) return;
    if (!input_read_double("Porcentaje urbano (0-100): ", &city_percent)) return;
    if (!input_read_double("Porcentaje carretera (0-100): ", &highway_percent)) return;
    if (!input_read_double("Precio del combustible por litro (DOP): ", &fuel_price)) return;

    if (calculator_compute(&store->items[index], distance, city_percent, highway_percent,
                           fuel_price, &result, error, sizeof(error)) != CALC_OK) {
        printf("Error: %s\n", error);
        return;
    }

    printf("\nResultado del viaje\n");
    printf("  Costo total:   %.2f DOP\n", result.total_cost);
    printf("  Costo por km:  %.2f DOP/km\n", result.cost_per_km);
    printf("  Desglose:\n");
    printf("    Combustible:            %.2f DOP\n", result.fuel_cost);
    printf("    Neumaticos:             %.2f DOP\n", result.tires_cost);
    printf("    Mantenimiento:          %.2f DOP\n", result.maintenance_cost);
    printf("    Seguro prorrateado:     %.2f DOP\n", result.insurance_cost);
    printf("    Impuestos prorrateados: %.2f DOP\n", result.taxes_cost);
    printf("    Depreciacion prorr.:    %.2f DOP\n", result.depreciation_cost);
    printf("    Limpieza prorrateada:   %.2f DOP\n", result.cleaning_cost);
}

int main(void)
{
    VehicleStore store;
    char error[256];
    int loaded;
    int option;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    loaded = storage_load(STORAGE_DEFAULT_PATH, &store, error, sizeof(error));
    if (loaded < 0) {
        printf("Error al cargar los vehiculos: %s\n", error);
        return 1;
    }
    if (loaded > 0) {
        printf("Aviso: %s\n", error);
    }

    for (;;) {
        print_menu();
        if (!input_read_int_range("Opcion: ", 0, 5, &option)) {
            break;
        }
        switch (option) {
        case 0:
            printf("Hasta luego.\n");
            return 0;
        case 1:
            create_vehicle(&store);
            break;
        case 2:
            list_vehicles(&store);
            break;
        case 3:
            edit_vehicle(&store);
            break;
        case 4:
            delete_vehicle(&store);
            break;
        case 5:
            calculate_trip(&store);
            break;
        default:
            break;
        }
    }
    return 0;
}
