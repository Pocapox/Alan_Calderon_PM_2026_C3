#ifndef REPORT_H
#define REPORT_H

#include "models.h"

/**
 * @brief Genera un reporte formateado del estado de pago de un préstamo.
 * El llamador debe liberar el puntero devuelto con free().
 */
char *reporte_generar_texto(const SistemaPrestamos *sys, int prestamoId);

/**
 * @brief Exporta el reporte de estado de pago a un archivo de texto.
 */
int reporte_exportar_txt(const SistemaPrestamos *sys, int prestamoId, const char *ruta, char *errBuf, int errSize);

#endif /* REPORT_H */
