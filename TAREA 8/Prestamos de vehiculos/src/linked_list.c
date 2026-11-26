#include "linked_list.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void set_err(char *errBuf, int errSize, const char *msg) {
    if (errBuf && errSize > 0) {
        snprintf(errBuf, (size_t)errSize, "%s", msg ? msg : "");
    }
}

static int is_blank(const char *s) {
    if (!s) return 1;
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

static int str_contains_ci(const char *haystack, const char *needle) {
    if (!haystack || !needle || *needle == '\0') return 0;
    size_t nlen = strlen(needle);
    for (const char *p = haystack; *p; p++) {
        size_t i = 0;
        while (i < nlen && p[i] &&
               tolower((unsigned char)p[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == nlen) return 1;
    }
    return 0;
}

char *str_clone(const char *src) {
    const char *safe = src ? src : "";
    size_t len = strlen(safe);
    char *copy = (char *)malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, safe, len + 1);
    return copy;
}

void sistema_init(SistemaPrestamos *sys) {
    if (!sys) return;
    sys->vehiculos = NULL;
    sys->clientes = NULL;
    sys->prestamos = NULL;
    sys->nextVehiculoId = 1;
    sys->nextClienteId = 1;
    sys->nextPrestamoId = 1;
}

void meses_free(MesPagoNode *head) {
    while (head) {
        MesPagoNode *next = head->next;
        free(head->fechaPago);
        free(head);
        head = next;
    }
}

void abonos_free(AbonoNode *head) {
    while (head) {
        AbonoNode *next = head->next;
        free(head->fecha);
        free(head);
        head = next;
    }
}

void prestamo_free_node(PrestamoNode *node) {
    if (!node) return;
    meses_free(node->meses);
    abonos_free(node->abonos);
    free(node);
}

void sistema_free(SistemaPrestamos *sys) {
    if (!sys) return;

    VehiculoNode *v = sys->vehiculos;
    while (v) {
        VehiculoNode *next = v->next;
        free(v->marca);
        free(v->modelo);
        free(v);
        v = next;
    }
    sys->vehiculos = NULL;

    ClienteNode *c = sys->clientes;
    while (c) {
        ClienteNode *next = c->next;
        free(c->nombre);
        free(c->documento);
        free(c->telefono);
        free(c);
        c = next;
    }
    sys->clientes = NULL;

    PrestamoNode *p = sys->prestamos;
    while (p) {
        PrestamoNode *next = p->next;
        prestamo_free_node(p);
        p = next;
    }
    sys->prestamos = NULL;
}

int vehiculo_anio_valido(int anio) {
    time_t now = time(NULL);
    struct tm *tmNow = localtime(&now);
    int maxAnio = tmNow ? (tmNow->tm_year + 1900 + 1) : 2030;
    return anio >= 1950 && anio <= maxAnio;
}

VehiculoNode *vehiculo_agregar_con_id(SistemaPrestamos *sys, int id, const char *marca, const char *modelo, int anio) {
    if (!sys) return NULL;
    VehiculoNode *node = (VehiculoNode *)malloc(sizeof(VehiculoNode));
    if (!node) return NULL;

    node->id = id;
    node->marca = str_clone(marca);
    node->modelo = str_clone(modelo);
    node->anio = anio;
    node->next = NULL;

    if (!node->marca || !node->modelo) {
        free(node->marca);
        free(node->modelo);
        free(node);
        return NULL;
    }

    if (!sys->vehiculos) {
        sys->vehiculos = node;
    } else {
        VehiculoNode *cur = sys->vehiculos;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }

    if (id >= sys->nextVehiculoId) {
        sys->nextVehiculoId = id + 1;
    }
    return node;
}

VehiculoNode *vehiculo_agregar(SistemaPrestamos *sys, const char *marca, const char *modelo, int anio, char *errBuf, int errSize) {
    if (!sys) {
        set_err(errBuf, errSize, "Sistema no inicializado.");
        return NULL;
    }
    if (is_blank(marca)) {
        set_err(errBuf, errSize, "La marca del vehiculo es obligatoria.");
        return NULL;
    }
    if (is_blank(modelo)) {
        set_err(errBuf, errSize, "El modelo del vehiculo es obligatorio.");
        return NULL;
    }
    if (!vehiculo_anio_valido(anio)) {
        set_err(errBuf, errSize, "El anio del vehiculo esta fuera del rango valido (1950 - anio actual+1).");
        return NULL;
    }

    VehiculoNode *created = vehiculo_agregar_con_id(sys, sys->nextVehiculoId, marca, modelo, anio);
    if (!created) {
        set_err(errBuf, errSize, "Error de memoria al registrar el vehiculo.");
        return NULL;
    }
    set_err(errBuf, errSize, "");
    return created;
}

VehiculoNode *vehiculo_buscar_id(const SistemaPrestamos *sys, int id) {
    if (!sys) return NULL;
    for (VehiculoNode *cur = sys->vehiculos; cur; cur = cur->next) {
        if (cur->id == id) return cur;
    }
    return NULL;
}

int vehiculo_tiene_prestamo(const SistemaPrestamos *sys, int vehiculoId) {
    if (!sys) return 0;
    for (PrestamoNode *p = sys->prestamos; p; p = p->next) {
        if (p->vehiculoId == vehiculoId) return 1;
    }
    return 0;
}

int vehiculo_eliminar(SistemaPrestamos *sys, int id, char *errBuf, int errSize) {
    if (!sys) {
        set_err(errBuf, errSize, "Sistema no inicializado.");
        return 0;
    }
    if (vehiculo_tiene_prestamo(sys, id)) {
        set_err(errBuf, errSize, "No se puede eliminar: el vehiculo esta asociado a un prestamo.");
        return 0;
    }

    VehiculoNode *prev = NULL;
    VehiculoNode *cur = sys->vehiculos;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->next = cur->next;
            else sys->vehiculos = cur->next;
            free(cur->marca);
            free(cur->modelo);
            free(cur);
            set_err(errBuf, errSize, "");
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    set_err(errBuf, errSize, "Vehiculo no encontrado.");
    return 0;
}

ClienteNode *cliente_buscar_documento(const SistemaPrestamos *sys, const char *documento) {
    if (!sys || !documento) return NULL;
    for (ClienteNode *cur = sys->clientes; cur; cur = cur->next) {
        if (cur->documento && strcmp(cur->documento, documento) == 0) {
            return cur;
        }
    }
    return NULL;
}

ClienteNode *cliente_agregar_con_id(SistemaPrestamos *sys, int id, const char *nombre, const char *documento, const char *telefono) {
    if (!sys) return NULL;
    ClienteNode *node = (ClienteNode *)malloc(sizeof(ClienteNode));
    if (!node) return NULL;

    node->id = id;
    node->nombre = str_clone(nombre);
    node->documento = str_clone(documento);
    node->telefono = str_clone(telefono);
    node->next = NULL;

    if (!node->nombre || !node->documento || !node->telefono) {
        free(node->nombre);
        free(node->documento);
        free(node->telefono);
        free(node);
        return NULL;
    }

    if (!sys->clientes) {
        sys->clientes = node;
    } else {
        ClienteNode *cur = sys->clientes;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }

    if (id >= sys->nextClienteId) {
        sys->nextClienteId = id + 1;
    }
    return node;
}

ClienteNode *cliente_agregar(SistemaPrestamos *sys, const char *nombre, const char *documento, const char *telefono, char *errBuf, int errSize) {
    if (!sys) {
        set_err(errBuf, errSize, "Sistema no inicializado.");
        return NULL;
    }
    if (is_blank(nombre)) {
        set_err(errBuf, errSize, "El nombre del cliente es obligatorio.");
        return NULL;
    }
    if (is_blank(documento)) {
        set_err(errBuf, errSize, "El documento de identidad es obligatorio.");
        return NULL;
    }
    if (is_blank(telefono)) {
        set_err(errBuf, errSize, "El telefono de contacto es obligatorio.");
        return NULL;
    }
    if (cliente_buscar_documento(sys, documento)) {
        set_err(errBuf, errSize, "Ya existe un cliente registrado con ese documento.");
        return NULL;
    }

    ClienteNode *created = cliente_agregar_con_id(sys, sys->nextClienteId, nombre, documento, telefono);
    if (!created) {
        set_err(errBuf, errSize, "Error de memoria al registrar el cliente.");
        return NULL;
    }
    set_err(errBuf, errSize, "");
    return created;
}

ClienteNode *cliente_buscar_id(const SistemaPrestamos *sys, int id) {
    if (!sys) return NULL;
    for (ClienteNode *cur = sys->clientes; cur; cur = cur->next) {
        if (cur->id == id) return cur;
    }
    return NULL;
}

int cliente_coincide_criterio(const ClienteNode *cliente, const char *criterio) {
    if (!cliente) return 0;
    if (is_blank(criterio)) return 1;

    char idBuf[32];
    snprintf(idBuf, sizeof(idBuf), "%d", cliente->id);
    if (strcmp(idBuf, criterio) == 0) return 1;
    if (cliente->documento && str_contains_ci(cliente->documento, criterio)) return 1;
    if (cliente->nombre && str_contains_ci(cliente->nombre, criterio)) return 1;
    return 0;
}

int cliente_tiene_prestamo(const SistemaPrestamos *sys, int clienteId) {
    if (!sys) return 0;
    for (PrestamoNode *p = sys->prestamos; p; p = p->next) {
        if (p->clienteId == clienteId) return 1;
    }
    return 0;
}

int cliente_eliminar(SistemaPrestamos *sys, int id, char *errBuf, int errSize) {
    if (!sys) {
        set_err(errBuf, errSize, "Sistema no inicializado.");
        return 0;
    }
    if (cliente_tiene_prestamo(sys, id)) {
        set_err(errBuf, errSize, "No se puede eliminar: el cliente posee un prestamo activo.");
        return 0;
    }

    ClienteNode *prev = NULL;
    ClienteNode *cur = sys->clientes;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->next = cur->next;
            else sys->clientes = cur->next;
            free(cur->nombre);
            free(cur->documento);
            free(cur->telefono);
            free(cur);
            set_err(errBuf, errSize, "");
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    set_err(errBuf, errSize, "Cliente no encontrado.");
    return 0;
}
