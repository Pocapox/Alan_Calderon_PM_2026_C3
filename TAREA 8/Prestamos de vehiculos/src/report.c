#include "report.h"
#include "linked_list.h"
#include "loan_service.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} DynBuf;

static int dynbuf_appendf(DynBuf *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list copy;
    va_copy(copy, args);
    int needed = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);

    if (needed < 0) {
        va_end(args);
        return 0;
    }

    size_t req = buf->len + (size_t)needed + 1;
    if (req > buf->cap) {
        size_t newCap = buf->cap ? buf->cap * 2 : 1024;
        while (newCap < req) newCap *= 2;
        char *newData = (char *)realloc(buf->data, newCap);
        if (!newData) {
            va_end(args);
            return 0;
        }
        buf->data = newData;
        buf->cap = newCap;
    }

    vsnprintf(buf->data + buf->len, buf->cap - buf->len, fmt, args);
    buf->len += (size_t)needed;
    va_end(args);
    return 1;
}

char *reporte_generar_texto(const SistemaPrestamos *sys, int prestamoId) {
    if (!sys) return NULL;
    PrestamoNode *p = prestamo_buscar_id(sys, prestamoId);
    if (!p) return NULL;

    ClienteNode *c = cliente_buscar_id(sys, p->clienteId);
    VehiculoNode *v = vehiculo_buscar_id(sys, p->vehiculoId);

    int mesesPagados = 0;
    double totalCuotasPagadas = 0.0;
    double totalAbonado = 0.0;
    prestamo_obtener_resumen(p, &mesesPagados, &totalCuotasPagadas, &totalAbonado);

    DynBuf buf = {NULL, 0, 0};
    dynbuf_appendf(&buf, "============================================================\r\n");
    dynbuf_appendf(&buf, "         REPORTE DE ESTADO DE PAGO - PRESTAMO #%d\r\n", p->id);
    dynbuf_appendf(&buf, "============================================================\r\n");
    dynbuf_appendf(&buf, "Estado General   : %s\r\n", p->liquidado ? "LIQUIDADO" : "ACTIVO");
    dynbuf_appendf(&buf, "\r\n[CLIENTE]\r\n");
    if (c) {
        dynbuf_appendf(&buf, "  ID             : %d\r\n", c->id);
        dynbuf_appendf(&buf, "  Nombre         : %s\r\n", c->nombre);
        dynbuf_appendf(&buf, "  Documento      : %s\r\n", c->documento);
        dynbuf_appendf(&buf, "  Telefono       : %s\r\n", c->telefono);
    } else {
        dynbuf_appendf(&buf, "  (Cliente ID %d no encontrado)\r\n", p->clienteId);
    }

    dynbuf_appendf(&buf, "\r\n[VEHICULO]\r\n");
    if (v) {
        dynbuf_appendf(&buf, "  ID             : %d\r\n", v->id);
        dynbuf_appendf(&buf, "  Marca / Modelo : %s %s\r\n", v->marca, v->modelo);
        dynbuf_appendf(&buf, "  Anio           : %d\r\n", v->anio);
    } else {
        dynbuf_appendf(&buf, "  (Vehiculo ID %d no encontrado)\r\n", p->vehiculoId);
    }

    dynbuf_appendf(&buf, "\r\n[RESUMEN FINANCIERO]\r\n");
    dynbuf_appendf(&buf, "  Monto Financiado : $%.2f\r\n", p->montoFinanciado);
    dynbuf_appendf(&buf, "  Tasa Anual       : %.2f%%\r\n", p->tasaAnual);
    dynbuf_appendf(&buf, "  Plazo            : %d meses (%d pagados / %d no pagados)\r\n",
                   p->plazoMeses, mesesPagados, p->plazoMeses - mesesPagados);
    dynbuf_appendf(&buf, "  Cuota Mensual    : $%.2f\r\n", p->cuotaMensual);
    dynbuf_appendf(&buf, "  Pagado en Cuotas : $%.2f\r\n", totalCuotasPagadas);
    dynbuf_appendf(&buf, "  Total Abonado    : $%.2f\r\n", totalAbonado);
    dynbuf_appendf(&buf, "  SALDO PENDIENTE  : $%.2f\r\n", p->saldoPendiente);

    dynbuf_appendf(&buf, "\r\n[LISTA DE MESES]\r\n");
    dynbuf_appendf(&buf, "  Mes | Estado     | Cuota     | Capital   | Interes   | Fecha Pago\r\n");
    dynbuf_appendf(&buf, "  ----+------------+-----------+-----------+-----------+-----------\r\n");
    for (MesPagoNode *m = p->meses; m; m = m->next) {
        dynbuf_appendf(&buf, "  %-3d | %-10s | $%8.2f | $%8.2f | $%8.2f | %s\r\n",
                       m->numeroMes,
                       m->pagado ? "PAGADO" : "NO PAGADO",
                       m->montoCuota,
                       m->capitalCuota,
                       m->interesCuota,
                       (m->pagado && m->fechaPago && m->fechaPago[0]) ? m->fechaPago : "-");
    }

    dynbuf_appendf(&buf, "\r\n[HISTORIAL DE ABONOS A CAPITAL]\r\n");
    if (!p->abonos) {
        dynbuf_appendf(&buf, "  No se han registrado abonos.\r\n");
    } else {
        for (AbonoNode *a = p->abonos; a; a = a->next) {
            dynbuf_appendf(&buf, "  Abono #%d | Fecha: %s | Monto: $%.2f\r\n",
                           a->id, a->fecha ? a->fecha : "-", a->monto);
        }
    }
    dynbuf_appendf(&buf, "============================================================\r\n");

    return buf.data;
}

int reporte_exportar_txt(const SistemaPrestamos *sys, int prestamoId, const char *ruta, char *errBuf, int errSize) {
    if (!sys || !ruta || ruta[0] == '\0') {
        if (errBuf && errSize > 0) snprintf(errBuf, (size_t)errSize, "Ruta de exportacion invalida.");
        return 0;
    }
    char *texto = reporte_generar_texto(sys, prestamoId);
    if (!texto) {
        if (errBuf && errSize > 0) snprintf(errBuf, (size_t)errSize, "No se pudo generar el reporte para el prestamo.");
        return 0;
    }

    FILE *fp = fopen(ruta, "wb");
    if (!fp) {
        free(texto);
        if (errBuf && errSize > 0) snprintf(errBuf, (size_t)errSize, "No se pudo crear el archivo de reporte.");
        return 0;
    }

    size_t len = strlen(texto);
    size_t written = fwrite(texto, 1, len, fp);
    fclose(fp);
    free(texto);

    if (written != len) {
        if (errBuf && errSize > 0) snprintf(errBuf, (size_t)errSize, "Error al escribir el reporte en disco.");
        return 0;
    }
    if (errBuf && errSize > 0) errBuf[0] = '\0';
    return 1;
}
