#ifndef GUI_H
#define GUI_H

#include <windows.h>
#include "models.h"

#define DATA_JSON_PATH "data/prestamos.json"

int gui_run(HINSTANCE hInstance, int nCmdShow, SistemaPrestamos *sys);

#endif /* GUI_H */
