#include "loan_service.h"
#include "linked_list.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void set_err(char *errBuf, int errSize, const char *msg) {
    if (errBuf && errSize > 0) {
        snprintf(errBuf, (size_t)errSize, "%s", msg ? msg : "");
    }
}

void fecha_actual_iso(char *buf, int bufSize) {
    if (!buf || bufSize <= 0) return;
    time_t now = time(NULL);
    struct tm *tmNow = localtime(&now);
    if (tmNow) {
        snprintf(buf, (size_t)bufSize, "%04d-%02d-%02d",
                 tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday);
    } else {
        snprintf(buf, (size_t)bufSize, "2026-01-01");
    }
}

double prestamo_calcular_cuota(double montoFinanciado, double tasaAnual, int plazoMeses) {
    if (montoFinanciado <= 0.0 || plazoMeses <= 0) return 0.0;
    if (tasaAnual <= 0.0) {
        return montoFinanciado / (double)plazoMeses;
    }
    double i = tasaAnual / 12.0 / 100.0;
    double factor = pow(1.0 + i, (double)plazoMeses);
    return montoFinanciado * (i * factor) / (factor - 1.0);
}

static MesPagoNode *crear_cronograma_meses(double montoFinanciado, double tasaAnual, int plazoMeses, double cuotaMensual) {
    MesPagoNode *head = NULL;
    MesPagoNode *tail = NULL;
    double saldoTeorico = montoFinanciado;
    double i = (tasaAnual > 0.0) ? (tasaAnual / 12.0 / 100.0) : 0.0;

    for (int m = 1; m <= plazoMeses; m++) {
        MesPagoNode *node = (MesPagoNode *)malloc(sizeof(MesPagoNode));
        if (!node) {
            meses_free(head);
            return NULL;
        }
        double interes = saldoTeorico * i;
        double capital = cuotaMensual - interes;
        double montoCuota = cuotaMensual;

        if (m == plazoMeses) {
            capital = saldoTeorico;
            montoCuota = capital + interes;
        } else if (capital > saldoTeorico) {
            capital = saldoTeorico;
        }
        saldoTeorico -= capital;
        if (saldoTeorico < 0.0) saldoTeorico = 0.0;

        node->numeroMes = m;
        node->montoCuota = montoCuota;
        node->capitalCuota = capital;
        node->interesCuota = interes;
        node->pagado = 0;
        node->fechaPago = str_clone("");
        node->next = NULL;

        if (!node->fechaPago) {
            free(node);
            meses_free(head);
            return NULL;
        }

        if (!head) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }
    return head;
}

PrestamoNode *prestamo_crear(
    SistemaPrestamos *sys,
    int clienteId,
    int vehiculoId,
    double montoFinanciado,
    double tasaAnual,
    int plazoMeses,
    char *errBuf,
    int errSize
) {
    if (!sys) {
        set_err(errBuf, errSize, "Sistema no inicializado.");
        return NULL;
    }
    if (!cliente_buscar_id(sys, clienteId)) {
        set_err(errBuf, errSize, "El cliente seleccionado no existe.");
        return NULL;
    }
    if (!vehiculo_buscar_id(sys, vehiculoId)) {
        set_err(errBuf, errSize, "El vehiculo seleccionado no existe.");
        return NULL;
    }
    if (montoFinanciado <= 0.0) {
        set_err(errBuf, errSize, "El monto financiado debe ser mayor a 0.");
        return NULL;
    }
    if (tasaAnual < 0.0) {
        set_err(errBuf, errSize, "La tasa anual no puede ser negativa.");
        return NULL;
    }
    if (plazoMeses <= 0 || plazoMeses > 360) {
        set_err(errBuf, errSize, "El plazo en meses debe estar entre 1 y 360.");
        return NULL;
    }

    double cuota = prestamo_calcular_cuota(montoFinanciado, tasaAnual, plazoMeses);
    MesPagoNode *meses = crear_cronograma_meses(montoFinanciado, tasaAnual, plazoMeses, cuota);
    if (!meses) {
        set_err(errBuf, errSize, "Error de memoria al generar el cronograma de meses.");
        return NULL;
    }

    PrestamoNode *p = (PrestamoNode *)malloc(sizeof(PrestamoNode));
    if (!p) {
        meses_free(meses);
        set_err(errBuf, errSize, "Error de memoria al crear el prestamo.");
        return NULL;
    }

    p->id = sys->nextPrestamoId++;
    p->clienteId = clienteId;
    p->vehiculoId = vehiculoId;
    p->montoFinanciado = montoFinanciado;
    p->tasaAnual = tasaAnual;
    p->plazoMeses = plazoMeses;
    p->cuotaMensual = cuota;
    p->saldoPendiente = montoFinanciado;
    p->liquidado = 0;
    p->meses = meses;
    p->abonos = NULL;
    p->nextAbonoId = 1;
    p->next = NULL;

    if (!sys->prestamos) {
        sys->prestamos = p;
    } else {
        PrestamoNode *cur = sys->prestamos;
        while (cur->next) cur = cur->next;
        cur->next = p;
    }

    set_err(errBuf, errSize, "");
    return p;
}

PrestamoNode *prestamo_buscar_id(const SistemaPrestamos *sys, int id) {
    if (!sys) return NULL;
    for (PrestamoNode *cur = sys->prestamos; cur; cur = cur->next) {
        if (cur->id == id) return cur;
    }
    return NULL;
}

MesPagoNode *prestamo_buscar_mes(const PrestamoNode *p, int numeroMes) {
    if (!p) return NULL;
    for (MesPagoNode *m = p->meses; m; m = m->next) {
        if (m->numeroMes == numeroMes) return m;
    }
    return NULL;
}

void prestamo_recalcular_cuotas(PrestamoNode *p) {
    if (!p) return;

    int mesesNoPagados = 0;
    for (MesPagoNode *m = p->meses; m; m = m->next) {
        if (!m->pagado) {
            mesesNoPagados++;
        }
    }

    if (mesesNoPagados == 0 || p->saldoPendiente <= 0.0) {
        p->cuotaMensual = 0.0;
        for (MesPagoNode *m = p->meses; m; m = m->next) {
            if (!m->pagado) {
                m->montoCuota = 0.0;
                m->capitalCuota = 0.0;
                m->interesCuota = 0.0;
            }
        }
        return;
    }

    double nuevaCuota = prestamo_calcular_cuota(p->saldoPendiente, p->tasaAnual, mesesNoPagados);
    p->cuotaMensual = nuevaCuota;

    double saldoTeorico = p->saldoPendiente;
    double i = (p->tasaAnual > 0.0) ? (p->tasaAnual / 12.0 / 100.0) : 0.0;
    int idx = 0;

    for (MesPagoNode *m = p->meses; m; m = m->next) {
        if (m->pagado) continue;
        idx++;

        double interes = saldoTeorico * i;
        double capital = nuevaCuota - interes;
        double montoCuota = nuevaCuota;

        if (idx == mesesNoPagados) {
            capital = saldoTeorico;
            montoCuota = capital + interes;
        } else if (capital > saldoTeorico) {
            capital = saldoTeorico;
        }

        saldoTeorico -= capital;
        if (saldoTeorico < 0.0) saldoTeorico = 0.0;

        m->montoCuota = montoCuota;
        m->capitalCuota = capital;
        m->interesCuota = interes;
    }
}

void prestamo_recalcular_saldo(PrestamoNode *p) {
    if (!p) return;
    double capitalPagado = 0.0;
    for (MesPagoNode *m = p->meses; m; m = m->next) {
        if (m->pagado) {
            capitalPagado += m->capitalCuota;
        }
    }
    double totalAbonos = 0.0;
    for (AbonoNode *a = p->abonos; a; a = a->next) {
        totalAbonos += a->monto;
    }

    double saldo = p->montoFinanciado - capitalPagado - totalAbonos;
    if (saldo <= 0.01) {
        p->saldoPendiente = 0.0;
        p->liquidado = 1;
    } else {
        p->saldoPendiente = saldo;
        p->liquidado = 0;
    }
}

int prestamo_pagar_mes(PrestamoNode *p, int numeroMes, const char *fecha, char *errBuf, int errSize) {
    if (!p) {
        set_err(errBuf, errSize, "Prestamo no encontrado.");
        return 0;
    }
    MesPagoNode *m = prestamo_buscar_mes(p, numeroMes);
    if (!m) {
        set_err(errBuf, errSize, "Mes no encontrado en el cronograma.");
        return 0;
    }
    if (m->pagado) {
        set_err(errBuf, errSize, "El mes seleccionado ya fue pagado.");
        return 0;
    }
    return prestamo_marcar_mes(p, numeroMes, 1, fecha, errBuf, errSize);
}

int prestamo_marcar_mes(PrestamoNode *p, int numeroMes, int pagado, const char *fecha, char *errBuf, int errSize) {
    if (!p) {
        set_err(errBuf, errSize, "Prestamo no encontrado.");
        return 0;
    }
    MesPagoNode *m = prestamo_buscar_mes(p, numeroMes);
    if (!m) {
        set_err(errBuf, errSize, "Mes no encontrado en el cronograma.");
        return 0;
    }

    char fechaBuf[32];
    if (pagado) {
        if (fecha && fecha[0] != '\0') {
            snprintf(fechaBuf, sizeof(fechaBuf), "%s", fecha);
        } else {
            fecha_actual_iso(fechaBuf, sizeof(fechaBuf));
        }
    } else {
        fechaBuf[0] = '\0';
    }

    char *nuevaFecha = str_clone(fechaBuf);
    if (!nuevaFecha) {
        set_err(errBuf, errSize, "Error de memoria al actualizar la fecha de pago.");
        return 0;
    }
    free(m->fechaPago);
    m->fechaPago = nuevaFecha;
    m->pagado = pagado ? 1 : 0;

    prestamo_recalcular_saldo(p);
    if (!pagado) {
        prestamo_recalcular_cuotas(p);
    }
    set_err(errBuf, errSize, "");
    return 1;
}

int prestamo_agregar_abono(PrestamoNode *p, double monto, const char *fecha, char *errBuf, int errSize) {
    if (!p) {
        set_err(errBuf, errSize, "Prestamo no encontrado.");
        return 0;
    }
    if (monto <= 0.0) {
        set_err(errBuf, errSize, "El monto del abono debe ser mayor a 0.");
        return 0;
    }
    if (p->saldoPendiente <= 0.0) {
        set_err(errBuf, errSize, "El prestamo ya se encuentra liquidado (saldo 0).");
        return 0;
    }
    if (monto > p->saldoPendiente + 0.01) {
        set_err(errBuf, errSize, "El abono no puede superar el saldo pendiente actual.");
        return 0;
    }
    if (monto > p->saldoPendiente) {
        monto = p->saldoPendiente;
    }

    char fechaBuf[32];
    if (fecha && fecha[0] != '\0') {
        snprintf(fechaBuf, sizeof(fechaBuf), "%s", fecha);
    } else {
        fecha_actual_iso(fechaBuf, sizeof(fechaBuf));
    }

    AbonoNode *abono = (AbonoNode *)malloc(sizeof(AbonoNode));
    if (!abono) {
        set_err(errBuf, errSize, "Error de memoria al registrar el abono.");
        return 0;
    }
    abono->id = p->nextAbonoId++;
    abono->monto = monto;
    abono->fecha = str_clone(fechaBuf);
    abono->next = NULL;

    if (!abono->fecha) {
        free(abono);
        set_err(errBuf, errSize, "Error de memoria al guardar fecha del abono.");
        return 0;
    }

    if (!p->abonos) {
        p->abonos = abono;
    } else {
        AbonoNode *cur = p->abonos;
        while (cur->next) cur = cur->next;
        cur->next = abono;
    }

    prestamo_recalcular_saldo(p);
    prestamo_recalcular_cuotas(p);
    set_err(errBuf, errSize, "");
    return 1;
}

void prestamo_obtener_resumen(
    const PrestamoNode *p,
    int *mesesPagadosOut,
    double *totalCuotasPagadasOut,
    double *totalAbonadoOut
) {
    int mesesPagados = 0;
    double totalCuotas = 0.0;
    double totalAbonado = 0.0;

    if (p) {
        for (MesPagoNode *m = p->meses; m; m = m->next) {
            if (m->pagado) {
                mesesPagados++;
                totalCuotas += m->montoCuota;
            }
        }
        for (AbonoNode *a = p->abonos; a; a = a->next) {
            totalAbonado += a->monto;
        }
    }

    if (mesesPagadosOut) *mesesPagadosOut = mesesPagados;
    if (totalCuotasPagadasOut) *totalCuotasPagadasOut = totalCuotas;
    if (totalAbonadoOut) *totalAbonadoOut = totalAbonado;
}
