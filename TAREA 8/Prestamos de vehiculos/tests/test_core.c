#include "models.h"
#include "linked_list.h"
#include "loan_service.h"
#include "json_store.h"
#include "report.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_vehiculos_y_clientes(void) {
    SistemaPrestamos sys;
    sistema_init(&sys);
    char err[256];

    /* Vehiculos: validaciones y alta */
    assert(vehiculo_agregar(&sys, "", "Corolla", 2022, err, sizeof(err)) == NULL);
    assert(vehiculo_agregar(&sys, "Toyota", "", 2022, err, sizeof(err)) == NULL);
    assert(vehiculo_agregar(&sys, "Toyota", "Corolla", 1800, err, sizeof(err)) == NULL);

    VehiculoNode *v1 = vehiculo_agregar(&sys, "Toyota", "Corolla", 2023, err, sizeof(err));
    assert(v1 != NULL && v1->id == 1);
    VehiculoNode *v2 = vehiculo_agregar(&sys, "Honda", "Civic", 2024, err, sizeof(err));
    assert(v2 != NULL && v2->id == 2);
    assert(vehiculo_buscar_id(&sys, 2) == v2);
    assert(vehiculo_eliminar(&sys, 2, err, sizeof(err)) == 1);
    assert(vehiculo_buscar_id(&sys, 2) == NULL);

    /* Clientes: alta, duplicado, busqueda, borrado */
    ClienteNode *c1 = cliente_agregar(&sys, "Ana Maria Lopez", "001-1234567-8", "809-555-1000", err, sizeof(err));
    assert(c1 != NULL && c1->id == 1);
    assert(cliente_agregar(&sys, "Otro Nombre", "001-1234567-8", "809-555-9999", err, sizeof(err)) == NULL);

    ClienteNode *c2 = cliente_agregar(&sys, "Carlos Perez", "002-7654321-0", "809-555-2000", err, sizeof(err));
    assert(c2 != NULL && c2->id == 2);

    assert(cliente_coincide_criterio(c1, "1") == 1);
    assert(cliente_coincide_criterio(c1, "maria") == 1);
    assert(cliente_coincide_criterio(c1, "1234567") == 1);
    assert(cliente_coincide_criterio(c1, "inexistente") == 0);

    assert(cliente_eliminar(&sys, 2, err, sizeof(err)) == 1);
    assert(cliente_buscar_id(&sys, 2) == NULL);

    sistema_free(&sys);
}

static void test_prestamos_pagos_y_abonos(void) {
    SistemaPrestamos sys;
    sistema_init(&sys);
    char err[256];

    VehiculoNode *v = vehiculo_agregar(&sys, "Mazda", "CX-5", 2025, err, sizeof(err));
    ClienteNode *c = cliente_agregar(&sys, "Luis Gomez", "402-1112223-4", "829-111-2233", err, sizeof(err));
    assert(v && c);

    /* Prestamo tasa 0% */
    PrestamoNode *p0 = prestamo_crear(&sys, c->id, v->id, 12000.0, 0.0, 12, err, sizeof(err));
    assert(p0 != NULL);
    assert(fabs(p0->cuotaMensual - 1000.0) < 0.01);
    assert(fabs(p0->saldoPendiente - 12000.0) < 0.01);

    /* Proteccion de borrado cuando hay prestamo */
    assert(vehiculo_eliminar(&sys, v->id, err, sizeof(err)) == 0);
    assert(cliente_eliminar(&sys, c->id, err, sizeof(err)) == 0);

    /* Prestamo tasa > 0% */
    PrestamoNode *p1 = prestamo_crear(&sys, c->id, v->id, 10000.0, 12.0, 12, err, sizeof(err));
    assert(p1 != NULL);
    assert(p1->cuotaMensual > 888.0 && p1->cuotaMensual < 889.0);

    /* Pagar mes 1 y evitar doble pago */
    assert(prestamo_pagar_mes(p1, 1, "2026-11-25", err, sizeof(err)) == 1);
    assert(prestamo_pagar_mes(p1, 1, "2026-11-25", err, sizeof(err)) == 0);
    assert(p1->saldoPendiente < 10000.0);

    /* Revertir mes 1 a no pagado */
    assert(prestamo_marcar_mes(p1, 1, 0, NULL, err, sizeof(err)) == 1);
    assert(fabs(p1->saldoPendiente - 10000.0) < 0.01);

    /* Re-pagar mes 1 y aplicar abono */
    assert(prestamo_pagar_mes(p1, 1, "2026-11-25", err, sizeof(err)) == 1);
    double saldoAntesAbono = p1->saldoPendiente;

    assert(prestamo_agregar_abono(p1, 0.0, "2026-11-26", err, sizeof(err)) == 0);
    assert(prestamo_agregar_abono(p1, saldoAntesAbono + 500.0, "2026-11-26", err, sizeof(err)) == 0);
    assert(prestamo_agregar_abono(p1, 2000.0, "2026-11-26", err, sizeof(err)) == 1);
    assert(fabs(p1->saldoPendiente - (saldoAntesAbono - 2000.0)) < 0.01);

    /* Liquidar con abono por el saldo restante */
    assert(prestamo_agregar_abono(p1, p1->saldoPendiente, "2026-11-27", err, sizeof(err)) == 1);
    assert(fabs(p1->saldoPendiente) < 0.01);
    assert(p1->liquidado == 1);

    sistema_free(&sys);
}

static void test_json_y_reporte(void) {
    SistemaPrestamos sys;
    sistema_init(&sys);
    char err[256];

    /* Carga de archivo inexistente no debe fallar */
    assert(json_cargar(&sys, "data/no_existe_test.json", err, sizeof(err)) == 1);

    VehiculoNode *v = vehiculo_agregar(&sys, "Kia", "Sportage", 2024, err, sizeof(err));
    ClienteNode *c = cliente_agregar(&sys, "Marta Diaz", "001-9988776-5", "849-222-3344", err, sizeof(err));
    PrestamoNode *p = prestamo_crear(&sys, c->id, v->id, 24000.0, 9.5, 24, err, sizeof(err));
    assert(v && c && p);

    assert(prestamo_pagar_mes(p, 1, "2026-11-01", err, sizeof(err)) == 1);
    assert(prestamo_pagar_mes(p, 2, "2026-12-01", err, sizeof(err)) == 1);
    assert(prestamo_agregar_abono(p, 3000.0, "2026-12-10", err, sizeof(err)) == 1);
    double saldoEsperado = p->saldoPendiente;

    const char *jsonPath = "data/test_prestamos.json";
    assert(json_guardar(&sys, jsonPath, err, sizeof(err)) == 1);

    /* Recargar en otro SistemaPrestamos */
    SistemaPrestamos sysLoaded;
    sistema_init(&sysLoaded);
    assert(json_cargar(&sysLoaded, jsonPath, err, sizeof(err)) == 1);

    VehiculoNode *vLoaded = vehiculo_buscar_id(&sysLoaded, v->id);
    ClienteNode *cLoaded = cliente_buscar_id(&sysLoaded, c->id);
    PrestamoNode *pLoaded = prestamo_buscar_id(&sysLoaded, p->id);
    assert(vLoaded != NULL && strcmp(vLoaded->marca, "Kia") == 0);
    assert(cLoaded != NULL && strcmp(cLoaded->nombre, "Marta Diaz") == 0);
    assert(pLoaded != NULL);
    assert(fabs(pLoaded->saldoPendiente - saldoEsperado) < 0.01);

    MesPagoNode *m2 = prestamo_buscar_mes(pLoaded, 2);
    MesPagoNode *m3 = prestamo_buscar_mes(pLoaded, 3);
    assert(m2 != NULL && m2->pagado == 1);
    assert(m3 != NULL && m3->pagado == 0);

    /* Reporte en texto y exportacion */
    char *rep = reporte_generar_texto(&sysLoaded, pLoaded->id);
    assert(rep != NULL);
    assert(strstr(rep, "Marta Diaz") != NULL);
    assert(strstr(rep, "Kia Sportage") != NULL);
    assert(strstr(rep, "PAGADO") != NULL);
    assert(strstr(rep, "NO PAGADO") != NULL);
    assert(strstr(rep, "SALDO PENDIENTE") != NULL);
    free(rep);

    const char *repPath = "data/test_reporte.txt";
    assert(reporte_exportar_txt(&sysLoaded, pLoaded->id, repPath, err, sizeof(err)) == 1);
    FILE *fp = fopen(repPath, "rb");
    assert(fp != NULL);
    fclose(fp);

    remove(jsonPath);
    remove(repPath);
    sistema_free(&sys);
    sistema_free(&sysLoaded);
}

static void test_abonos_detallado(void) {
    SistemaPrestamos sys;
    sistema_init(&sys);
    char err[256];

    VehiculoNode *v = vehiculo_agregar(&sys, "Nissan", "Sentra", 2024, err, sizeof(err));
    ClienteNode *c = cliente_agregar(&sys, "Pedro Ruiz", "001-3334445-6", "809-333-4455", err, sizeof(err));
    PrestamoNode *p = prestamo_crear(&sys, c->id, v->id, 10000.0, 12.0, 12, err, sizeof(err));
    assert(v && c && p);

    double cuotaOriginal = p->cuotaMensual;

    /* Pagar mes 1 antes de abonar para verificar que los meses pagados no cambien */
    assert(prestamo_pagar_mes(p, 1, "2026-10-15", err, sizeof(err)) == 1);
    double cuotaMes1Pagado = prestamo_buscar_mes(p, 1)->montoCuota;
    double saldoTrasMes1 = p->saldoPendiente;

    /* Multiples abonos consecutivos y actualizacion de cuotas */
    assert(prestamo_agregar_abono(p, 1500.0, "2026-11-01", err, sizeof(err)) == 1);
    assert(fabs(p->saldoPendiente - (saldoTrasMes1 - 1500.0)) < 0.01);
    assert(p->cuotaMensual < cuotaOriginal);

    /* Mes 1 pagado conserva su cuota original; meses no pagados tienen nueva cuota */
    assert(fabs(prestamo_buscar_mes(p, 1)->montoCuota - cuotaMes1Pagado) < 0.01);
    assert(fabs(prestamo_buscar_mes(p, 2)->montoCuota - p->cuotaMensual) < 0.01);

    assert(prestamo_agregar_abono(p, 2500.0, "", err, sizeof(err)) == 1);
    double saldoEsperado = saldoTrasMes1 - 4000.0;
    assert(fabs(p->saldoPendiente - saldoEsperado) < 0.01);
    double cuotaTrasSegundoAbono = p->cuotaMensual;
    assert(fabs(prestamo_buscar_mes(p, 2)->montoCuota - cuotaTrasSegundoAbono) < 0.01);

    int mesesPagados = 0;
    double totalCuotas = 0.0, totalAbonado = 0.0;
    prestamo_obtener_resumen(p, &mesesPagados, &totalCuotas, &totalAbonado);
    assert(mesesPagados == 1);
    assert(fabs(totalAbonado - 4000.0) < 0.01);

    /* Guardar y recargar JSON verificando lista de abonos, cuotas actualizadas y saldo */
    const char *jsonPath = "data/test_abonos.json";
    assert(json_guardar(&sys, jsonPath, err, sizeof(err)) == 1);

    SistemaPrestamos loaded;
    sistema_init(&loaded);
    assert(json_cargar(&loaded, jsonPath, err, sizeof(err)) == 1);
    PrestamoNode *pLoaded = prestamo_buscar_id(&loaded, p->id);
    assert(pLoaded != NULL);
    assert(fabs(pLoaded->saldoPendiente - saldoEsperado) < 0.01);
    assert(fabs(pLoaded->cuotaMensual - cuotaTrasSegundoAbono) < 0.01);
    assert(fabs(prestamo_buscar_mes(pLoaded, 2)->montoCuota - cuotaTrasSegundoAbono) < 0.01);
    assert(pLoaded->abonos != NULL && pLoaded->abonos->id == 1);
    assert(pLoaded->abonos->next != NULL && pLoaded->abonos->next->id == 2);

    /* Reporte contiene ambos abonos y saldo actualizado */
    char *rep = reporte_generar_texto(&loaded, pLoaded->id);
    assert(rep != NULL);
    assert(strstr(rep, "Abono #1") != NULL);
    assert(strstr(rep, "Abono #2") != NULL);
    assert(strstr(rep, "$4000.00") != NULL);
    free(rep);

    /* Liquidar saldo restante: las cuotas pendientes quedan en 0 y rechaza nuevos abonos */
    assert(prestamo_agregar_abono(pLoaded, pLoaded->saldoPendiente, "2026-11-15", err, sizeof(err)) == 1);
    assert(pLoaded->liquidado == 1);
    assert(fabs(pLoaded->saldoPendiente) < 0.01);
    assert(fabs(pLoaded->cuotaMensual) < 0.01);
    assert(fabs(prestamo_buscar_mes(pLoaded, 2)->montoCuota) < 0.01);
    assert(prestamo_agregar_abono(pLoaded, 100.0, "2026-11-16", err, sizeof(err)) == 0);

    remove(jsonPath);
    sistema_free(&sys);
    sistema_free(&loaded);
}

int main(void) {
    test_vehiculos_y_clientes();
    test_prestamos_pagos_y_abonos();
    test_json_y_reporte();
    test_abonos_detallado();
    printf("Todas las pruebas unitarias pasaron exitosamente.\n");
    return 0;
}
