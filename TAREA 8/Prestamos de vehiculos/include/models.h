#ifndef MODELS_H
#define MODELS_H

/**
 * @brief Nodo de lista enlazada para un vehículo en el catálogo.
 */
typedef struct VehiculoNode {
    int id;
    char *marca;
    char *modelo;
    int anio;
    struct VehiculoNode *next;
} VehiculoNode;

/**
 * @brief Nodo de lista enlazada para un cliente.
 */
typedef struct ClienteNode {
    int id;
    char *nombre;
    char *documento;
    char *telefono;
    struct ClienteNode *next;
} ClienteNode;

/**
 * @brief Nodo de lista enlazada para cada mes del cronograma de pago.
 */
typedef struct MesPagoNode {
    int numeroMes;
    double montoCuota;
    double capitalCuota;
    double interesCuota;
    int pagado;       /* 0 = no pagado, 1 = pagado */
    char *fechaPago;  /* Cadena vacía si no está pagado */
    struct MesPagoNode *next;
} MesPagoNode;

/**
 * @brief Nodo de lista enlazada para pagos de abono a capital.
 */
typedef struct AbonoNode {
    int id;
    double monto;
    char *fecha;
    struct AbonoNode *next;
} AbonoNode;

/**
 * @brief Nodo de lista enlazada para un préstamo vehicular.
 */
typedef struct PrestamoNode {
    int id;
    int clienteId;
    int vehiculoId;
    double montoFinanciado;
    double tasaAnual;
    int plazoMeses;
    double cuotaMensual;
    double saldoPendiente;
    int liquidado;    /* 1 si saldoPendiente == 0 */
    MesPagoNode *meses;
    AbonoNode *abonos;
    int nextAbonoId;
    struct PrestamoNode *next;
} PrestamoNode;

/**
 * @brief Contenedor principal del sistema con todas las listas enlazadas.
 */
typedef struct SistemaPrestamos {
    VehiculoNode *vehiculos;
    ClienteNode *clientes;
    PrestamoNode *prestamos;
    int nextVehiculoId;
    int nextClienteId;
    int nextPrestamoId;
} SistemaPrestamos;

#endif /* MODELS_H */
