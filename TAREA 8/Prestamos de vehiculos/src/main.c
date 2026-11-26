#include <windows.h>
#include "models.h"
#include "linked_list.h"
#include "json_store.h"
#include "gui.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    SistemaPrestamos sys;
    sistema_init(&sys);

    char err[256];
    if (!json_cargar(&sys, DATA_JSON_PATH, err, sizeof(err))) {
        MessageBoxA(NULL, err, "Advertencia al cargar JSON", MB_OK | MB_ICONWARNING);
    }

    int exitCode = gui_run(hInstance, nCmdShow, &sys);

    if (!json_guardar(&sys, DATA_JSON_PATH, err, sizeof(err))) {
        MessageBoxA(NULL, err, "Error al guardar JSON", MB_OK | MB_ICONERROR);
    }

    sistema_free(&sys);
    return exitCode;
}
