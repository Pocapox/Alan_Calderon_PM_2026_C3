#ifndef VEHICLE_H
#define VEHICLE_H

#define MAX_VEHICLES 100
#define MAX_NAME_LEN 64
#define DEFAULT_DATA_FILE "vehiculos.csv"

typedef struct {
    char nombre[MAX_NAME_LEN];
    double precio_combustible;   // $ / Litro
    double consumo_ciudad;       // Litros / 100 km
    double consumo_carretera;    // Litros / 100 km
    double seguro_anual;         // $ / año
    double costo_neumaticos_km;  // $ / km
    double costo_mant_km;        // $ / km
    // Depreciación
    double valor_inicial;        // $ valor de compra
    double tiempo_uso_anios;     // años estimados de uso
    double valor_final;          // $ valor de reventa/residual
    double km_anuales;           // km estimados por año
    // Impuestos y limpieza
    double impuestos_anual;      // $ / año
    double costo_limpieza_anual; // $ / año
} Vehiculo;

typedef struct {
    double distancia_km;
    double pct_carretera;        // 0.0 a 100.0
    double pct_ciudad;           // 0.0 a 100.0
    double precio_combustible;   // $ / Litro para el viaje
} ParametrosViaje;

typedef struct {
    // Totales en el viaje ($)
    double costo_combustible;
    double costo_neumaticos;
    double costo_mantenimiento;
    double costo_depreciacion;
    double costo_seguro;
    double costo_impuestos;
    double costo_limpieza;
    double costo_total;

    // Costos por km ($/km)
    double km_combustible;
    double km_neumaticos;
    double km_mantenimiento;
    double km_depreciacion;
    double km_seguro;
    double km_impuestos;
    double km_limpieza;
    double costo_por_km;

    // Métricas adicionales de viaje
    double litros_consumidos;
    double consumo_mixto_l100km;
} ResultadoViaje;

// Funciones de gestión de vehículos
int cargar_vehiculos(Vehiculo vehiculos[], int max_v, const char* filepath);
int guardar_vehiculos(const Vehiculo vehiculos[], int total, const char* filepath);
void cargar_vehiculos_por_defecto(Vehiculo vehiculos[], int* total);

// Función de cálculo
int calcular_costo_viaje(const Vehiculo* v, const ParametrosViaje* viaje, ResultadoViaje* res);

#endif // VEHICLE_H
