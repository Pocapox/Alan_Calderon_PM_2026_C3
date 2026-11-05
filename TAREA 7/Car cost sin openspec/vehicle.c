#include "vehicle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cargar_vehiculos_por_defecto(Vehiculo vehiculos[], int* total) {
    *total = 0;

    // Vehículo 1: Sedán Compacto Moderno
    strncpy(vehiculos[0].nombre, "Toyota Corolla 2.0", MAX_NAME_LEN - 1);
    vehiculos[0].precio_combustible = 1.25;
    vehiculos[0].consumo_ciudad = 8.5;       // L / 100 km
    vehiculos[0].consumo_carretera = 5.8;    // L / 100 km
    vehiculos[0].seguro_anual = 800.00;      // $ / año
    vehiculos[0].costo_neumaticos_km = 0.025;// $ / km
    vehiculos[0].costo_mant_km = 0.040;      // $ / km
    vehiculos[0].valor_inicial = 24000.0;    // $
    vehiculos[0].tiempo_uso_anios = 5.0;     // 5 años
    vehiculos[0].valor_final = 12000.0;      // $ residual
    vehiculos[0].km_anuales = 15000.0;       // km / año
    vehiculos[0].impuestos_anual = 350.00;   // $ / año
    vehiculos[0].costo_limpieza_anual = 240.0;// $ / año ($20/mes)
    (*total)++;

    // Vehículo 2: SUV / Camioneta
    strncpy(vehiculos[1].nombre, "Ford Ranger 4x4", MAX_NAME_LEN - 1);
    vehiculos[1].precio_combustible = 1.20;
    vehiculos[1].consumo_ciudad = 11.5;
    vehiculos[1].consumo_carretera = 8.2;
    vehiculos[1].seguro_anual = 1200.00;
    vehiculos[1].costo_neumaticos_km = 0.045;
    vehiculos[1].costo_mant_km = 0.065;
    vehiculos[1].valor_inicial = 38000.0;
    vehiculos[1].tiempo_uso_anios = 6.0;
    vehiculos[1].valor_final = 17000.0;
    vehiculos[1].km_anuales = 20000.0;
    vehiculos[1].impuestos_anual = 500.00;
    vehiculos[1].costo_limpieza_anual = 360.0;
    (*total)++;

    // Vehículo 3: Compacto Económico
    strncpy(vehiculos[2].nombre, "Nissan Versa 1.6", MAX_NAME_LEN - 1);
    vehiculos[2].precio_combustible = 1.25;
    vehiculos[2].consumo_ciudad = 7.2;
    vehiculos[2].consumo_carretera = 5.1;
    vehiculos[2].seguro_anual = 600.00;
    vehiculos[2].costo_neumaticos_km = 0.020;
    vehiculos[2].costo_mant_km = 0.030;
    vehiculos[2].valor_inicial = 16000.0;
    vehiculos[2].tiempo_uso_anios = 5.0;
    vehiculos[2].valor_final = 8500.0;
    vehiculos[2].km_anuales = 12000.0;
    vehiculos[2].impuestos_anual = 220.00;
    vehiculos[2].costo_limpieza_anual = 180.0;
    (*total)++;
}

int guardar_vehiculos(const Vehiculo vehiculos[], int total, const char* filepath) {
    FILE* f = fopen(filepath, "w");
    if (!f) return 0;

    fprintf(f, "# Nombre;PrecioCombustible;ConsumoCiudad;ConsumoCarretera;SeguroAnual;NeumaticosKm;MantKm;ValorInicial;TiempoUsoAnios;ValorFinal;KmAnuales;ImpuestosAnual;LimpiezaAnual\n");
    for (int i = 0; i < total; i++) {
        fprintf(f, "%s;%.4f;%.2f;%.2f;%.2f;%.4f;%.4f;%.2f;%.2f;%.2f;%.1f;%.2f;%.2f\n",
            vehiculos[i].nombre,
            vehiculos[i].precio_combustible,
            vehiculos[i].consumo_ciudad,
            vehiculos[i].consumo_carretera,
            vehiculos[i].seguro_anual,
            vehiculos[i].costo_neumaticos_km,
            vehiculos[i].costo_mant_km,
            vehiculos[i].valor_inicial,
            vehiculos[i].tiempo_uso_anios,
            vehiculos[i].valor_final,
            vehiculos[i].km_anuales,
            vehiculos[i].impuestos_anual,
            vehiculos[i].costo_limpieza_anual);
    }

    fclose(f);
    return 1;
}

int cargar_vehiculos(Vehiculo vehiculos[], int max_v, const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) return -1; // archivo no existe

    char line[512];
    int count = 0;

    while (fgets(line, sizeof(line), f) && count < max_v) {
        // Ignorar comentarios o líneas vacías
        if (line[0] == '#' || line[0] == '\r' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        // Remover salto de línea al final
        line[strcspn(line, "\r\n")] = 0;

        Vehiculo v;
        memset(&v, 0, sizeof(Vehiculo));

        char* token = strtok(line, ";");
        if (!token) continue;
        strncpy(v.nombre, token, MAX_NAME_LEN - 1);

        token = strtok(NULL, ";"); if (token) v.precio_combustible = atof(token);
        token = strtok(NULL, ";"); if (token) v.consumo_ciudad = atof(token);
        token = strtok(NULL, ";"); if (token) v.consumo_carretera = atof(token);
        token = strtok(NULL, ";"); if (token) v.seguro_anual = atof(token);
        token = strtok(NULL, ";"); if (token) v.costo_neumaticos_km = atof(token);
        token = strtok(NULL, ";"); if (token) v.costo_mant_km = atof(token);
        token = strtok(NULL, ";"); if (token) v.valor_inicial = atof(token);
        token = strtok(NULL, ";"); if (token) v.tiempo_uso_anios = atof(token);
        token = strtok(NULL, ";"); if (token) v.valor_final = atof(token);
        token = strtok(NULL, ";"); if (token) v.km_anuales = atof(token);
        token = strtok(NULL, ";"); if (token) v.impuestos_anual = atof(token);
        token = strtok(NULL, ";"); if (token) v.costo_limpieza_anual = atof(token);

        vehiculos[count++] = v;
    }

    fclose(f);
    return count;
}

int calcular_costo_viaje(const Vehiculo* v, const ParametrosViaje* viaje, ResultadoViaje* res) {
    if (!v || !viaje || !res) return 0;
    memset(res, 0, sizeof(ResultadoViaje));

    if (viaje->distancia_km <= 0.0) {
        return 0;
    }

    double dist_total = viaje->distancia_km;

    // Normalizar porcentajes de carretera y ciudad si no suman 100
    double pct_carr = viaje->pct_carretera;
    double pct_ciud = viaje->pct_ciudad;
    double suma_pct = pct_carr + pct_ciud;
    if (suma_pct > 0.0001) {
        pct_carr = (pct_carr / suma_pct) * 100.0;
        pct_ciud = (pct_ciud / suma_pct) * 100.0;
    } else {
        pct_carr = 50.0;
        pct_ciud = 50.0;
    }

    // 1. Combustible
    double dist_carr = dist_total * (pct_carr / 100.0);
    double dist_ciud = dist_total * (pct_ciud / 100.0);
    double litros_carr = dist_carr * (v->consumo_carretera / 100.0);
    double litros_ciud = dist_ciud * (v->consumo_ciudad / 100.0);
    res->litros_consumidos = litros_carr + litros_ciud;
    res->consumo_mixto_l100km = (res->litros_consumidos * 100.0) / dist_total;

    double precio_comb = (viaje->precio_combustible > 0) ? viaje->precio_combustible : v->precio_combustible;
    res->costo_combustible = res->litros_consumidos * precio_comb;
    res->km_combustible = res->costo_combustible / dist_total;

    // 2. Neumáticos
    res->km_neumaticos = v->costo_neumaticos_km;
    res->costo_neumaticos = res->km_neumaticos * dist_total;

    // 3. Mantenimiento
    res->km_mantenimiento = v->costo_mant_km;
    res->costo_mantenimiento = res->km_mantenimiento * dist_total;

    // Kilometraje anual de referencia para prorratear costos fijos anuales
    double km_anual = (v->km_anuales > 0) ? v->km_anuales : 15000.0;

    // 4. Depreciación: (Valor Inicial - Valor Final) / Tiempo de Uso / Km Anuales
    double dep_total = v->valor_inicial - v->valor_final;
    if (dep_total < 0.0) dep_total = 0.0;
    double anios = (v->tiempo_uso_anios > 0) ? v->tiempo_uso_anios : 1.0;
    double dep_anual = dep_total / anios;
    res->km_depreciacion = dep_anual / km_anual;
    res->costo_depreciacion = res->km_depreciacion * dist_total;

    // 5. Seguro por año
    res->km_seguro = v->seguro_anual / km_anual;
    res->costo_seguro = res->km_seguro * dist_total;

    // 6. Impuestos por año
    res->km_impuestos = v->impuestos_anual / km_anual;
    res->costo_impuestos = res->km_impuestos * dist_total;

    // 7. Costo Limpieza
    res->km_limpieza = v->costo_limpieza_anual / km_anual;
    res->costo_limpieza = res->km_limpieza * dist_total;

    // Totales
    res->costo_total = res->costo_combustible +
                       res->costo_neumaticos +
                       res->costo_mantenimiento +
                       res->costo_depreciacion +
                       res->costo_seguro +
                       res->costo_impuestos +
                       res->costo_limpieza;

    res->costo_por_km = res->costo_total / dist_total;

    return 1;
}
