#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "models.h"

/* Inicialización y liberación completa */
void sistema_init(SistemaPrestamos *sys);
void sistema_free(SistemaPrestamos *sys);

/* Utilidad de memoria dinámica para cadenas */
char *str_clone(const char *src);

/* Operaciones de vehículos */
int vehiculo_anio_valido(int anio);
VehiculoNode *vehiculo_agregar(SistemaPrestamos *sys, const char *marca, const char *modelo, int anio, char *errBuf, int errSize);
VehiculoNode *vehiculo_agregar_con_id(SistemaPrestamos *sys, int id, const char *marca, const char *modelo, int anio);
VehiculoNode *vehiculo_buscar_id(const SistemaPrestamos *sys, int id);
int vehiculo_tiene_prestamo(const SistemaPrestamos *sys, int vehiculoId);
int vehiculo_eliminar(SistemaPrestamos *sys, int id, char *errBuf, int errSize);

/* Operaciones de clientes */
ClienteNode *cliente_agregar(SistemaPrestamos *sys, const char *nombre, const char *documento, const char *telefono, char *errBuf, int errSize);
ClienteNode *cliente_agregar_con_id(SistemaPrestamos *sys, int id, const char *nombre, const char *documento, const char *telefono);
ClienteNode *cliente_buscar_id(const SistemaPrestamos *sys, int id);
ClienteNode *cliente_buscar_documento(const SistemaPrestamos *sys, const char *documento);
int cliente_coincide_criterio(const ClienteNode *cliente, const char *criterio);
int cliente_tiene_prestamo(const SistemaPrestamos *sys, int clienteId);
int cliente_eliminar(SistemaPrestamos *sys, int id, char *errBuf, int errSize);

/* Liberación de listas anidadas */
void meses_free(MesPagoNode *head);
void abonos_free(AbonoNode *head);
void prestamo_free_node(PrestamoNode *node);

#endif /* LINKED_LIST_H */
