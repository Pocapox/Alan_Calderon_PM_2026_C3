#ifndef DLG_VEHICLE_H
#define DLG_VEHICLE_H

#include <windows.h>
#include "vehicle.h"

// Retorna 1 si el usuario guardó los cambios, 0 si canceló
int MostrarDialogoVehiculo(HWND hwndParent, Vehiculo* vehiculo, int es_edicion);

#endif // DLG_VEHICLE_H
