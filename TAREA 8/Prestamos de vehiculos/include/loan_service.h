#ifndef LOAN_SERVICE_H
#define LOAN_SERVICE_H

#include "models.h"

double prestamo_calcular_cuota(double montoFinanciado, double tasaAnual, int plazoMeses);

PrestamoNode *prestamo_crear(
    SistemaPrestamos *sys,
    int clienteId,
    int vehiculoId,
    double montoFinanciado,
    double tasaAnual,
    int plazoMeses,
    char *errBuf,
    int errSize
);

PrestamoNode *prestamo_buscar_id(const SistemaPrestamos *sys, int id);
MesPagoNode *prestamo_buscar_mes(const PrestamoNode *p, int numeroMes);

void prestamo_recalcular_saldo(PrestamoNode *p);
void prestamo_recalcular_cuotas(PrestamoNode *p);

int prestamo_pagar_mes(PrestamoNode *p, int numeroMes, const char *fecha, char *errBuf, int errSize);
int prestamo_marcar_mes(PrestamoNode *p, int numeroMes, int pagado, const char *fecha, char *errBuf, int errSize);
int prestamo_agregar_abono(PrestamoNode *p, double monto, const char *fecha, char *errBuf, int errSize);

void prestamo_obtener_resumen(
    const PrestamoNode *p,
    int *mesesPagadosOut,
    double *totalCuotasPagadasOut,
    double *totalAbonadoOut
);

void fecha_actual_iso(char *buf, int bufSize);

#endif /* LOAN_SERVICE_H */
