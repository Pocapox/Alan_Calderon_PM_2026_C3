#define _WIN32_WINNT 0x0600
#include "dlg_vehicle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    HWND hDlg;
    HWND hTxtNombre;
    HWND hTxtPrecioComb;
    HWND hTxtConsCiudad;
    HWND hTxtConsCarretera;
    HWND hTxtNeumaticos;
    HWND hTxtMantenimiento;
    HWND hTxtValorInicial;
    HWND hTxtValorFinal;
    HWND hTxtTiempoAnios;
    HWND hTxtKmAnuales;
    HWND hTxtSeguro;
    HWND hTxtImpuestos;
    HWND hTxtLimpieza;
    HWND hBtnGuardar;
    HWND hBtnCancelar;
    HFONT hFont;
    Vehiculo* pVehiculo;
    int es_edicion;
    int guardado;
} DlgContext;

static double parse_double(const char* str) {
    char buf[64];
    strncpy(buf, str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    for (int i = 0; buf[i]; i++) {
        if (buf[i] == ',') buf[i] = '.';
    }
    return atof(buf);
}

static void set_edit_double(HWND hEdit, double val, const char* fmt) {
    char buf[64];
    snprintf(buf, sizeof(buf), fmt, val);
    SetWindowTextA(hEdit, buf);
}

static LRESULT CALLBACK VehicleDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    DlgContext* ctx = (DlgContext*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        ctx = (DlgContext*)cs->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ctx);
        ctx->hDlg = hWnd;

        ctx->hFont = CreateFontA(
            -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        int y = 15;
        const int LBL_X = 25;
        const int LBL_W = 210;
        const int EDT_X = 240;
        const int EDT_W = 240;
        const int ROW_H = 24;
        const int GAP = 8;

        // --- Grupo 1: Datos Generales y Combustible ---
        HWND hGrp1 = CreateWindowExA(0, "BUTTON", " 1. Identificación y Combustible ",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            15, y, 485, 150, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hGrp1, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += 24;
        HWND hLbl = CreateWindowExA(0, "STATIC", "Nombre / Modelo del Vehículo:",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtNombre = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtNombre, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Precio Combustible Ref. ($/Litro):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtPrecioComb = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1.25",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtPrecioComb, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Consumo en Ciudad (L / 100 km):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtConsCiudad = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "8.50",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtConsCiudad, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Consumo en Carretera (L / 100 km):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtConsCarretera = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "6.00",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtConsCarretera, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        // --- Grupo 2: Costos por Kilómetro ---
        y += ROW_H + 20;
        HWND hGrp2 = CreateWindowExA(0, "BUTTON", " 2. Costos de Desgaste por KM ",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            15, y, 485, 90, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hGrp2, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += 24;
        hLbl = CreateWindowExA(0, "STATIC", "Costo Neumáticos por KM ($/km):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtNeumaticos = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0.025",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtNeumaticos, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Costo Mantenimiento por KM ($/km):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtMantenimiento = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0.040",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtMantenimiento, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        // --- Grupo 3: Depreciación ---
        y += ROW_H + 20;
        HWND hGrp3 = CreateWindowExA(0, "BUTTON", " 3. Depreciación del Vehículo ",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            15, y, 485, 150, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hGrp3, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += 24;
        hLbl = CreateWindowExA(0, "STATIC", "Valor Inicial / Compra ($):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtValorInicial = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "20000.00",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtValorInicial, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Tiempo de Uso Estimado (años):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtTiempoAnios = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "5.0",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtTiempoAnios, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Valor Final / Reventa Residual ($):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtValorFinal = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "10000.00",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtValorFinal, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Kilómetros Anuales Estimados (km/año):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtKmAnuales = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "15000",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtKmAnuales, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        // --- Grupo 4: Costos Fijos Anuales ---
        y += ROW_H + 20;
        HWND hGrp4 = CreateWindowExA(0, "BUTTON", " 4. Costos Fijos Anuales ",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            15, y, 485, 122, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hGrp4, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += 24;
        hLbl = CreateWindowExA(0, "STATIC", "Costo Seguro por Año ($/año):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtSeguro = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "700.00",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtSeguro, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Costos Impuestos / Patente ($/año):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtImpuestos = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "300.00",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtImpuestos, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        y += ROW_H + GAP;
        hLbl = CreateWindowExA(0, "STATIC", "Costo Limpieza y Lavado ($/año):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, LBL_X, y + 2, LBL_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);
        ctx->hTxtLimpieza = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "240.00",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, EDT_X, y, EDT_W, ROW_H, hWnd, NULL, cs->hInstance, NULL);
        SendMessage(ctx->hTxtLimpieza, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        // --- Botones de Acción ---
        y += ROW_H + 25;
        ctx->hBtnGuardar = CreateWindowExA(0, "BUTTON", " Guardar Vehículo ",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            120, y, 160, 32, hWnd, (HMENU)IDOK, cs->hInstance, NULL);
        SendMessage(ctx->hBtnGuardar, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        ctx->hBtnCancelar = CreateWindowExA(0, "BUTTON", " Cancelar ",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            300, y, 140, 32, hWnd, (HMENU)IDCANCEL, cs->hInstance, NULL);
        SendMessage(ctx->hBtnCancelar, WM_SETFONT, (WPARAM)ctx->hFont, TRUE);

        // Si es edición, rellenar campos con valores actuales
        if (ctx->es_edicion && ctx->pVehiculo) {
            SetWindowTextA(ctx->hTxtNombre, ctx->pVehiculo->nombre);
            set_edit_double(ctx->hTxtPrecioComb, ctx->pVehiculo->precio_combustible, "%.3f");
            set_edit_double(ctx->hTxtConsCiudad, ctx->pVehiculo->consumo_ciudad, "%.2f");
            set_edit_double(ctx->hTxtConsCarretera, ctx->pVehiculo->consumo_carretera, "%.2f");
            set_edit_double(ctx->hTxtNeumaticos, ctx->pVehiculo->costo_neumaticos_km, "%.4f");
            set_edit_double(ctx->hTxtMantenimiento, ctx->pVehiculo->costo_mant_km, "%.4f");
            set_edit_double(ctx->hTxtValorInicial, ctx->pVehiculo->valor_inicial, "%.2f");
            set_edit_double(ctx->hTxtValorFinal, ctx->pVehiculo->valor_final, "%.2f");
            set_edit_double(ctx->hTxtTiempoAnios, ctx->pVehiculo->tiempo_uso_anios, "%.1f");
            set_edit_double(ctx->hTxtKmAnuales, ctx->pVehiculo->km_anuales, "%.0f");
            set_edit_double(ctx->hTxtSeguro, ctx->pVehiculo->seguro_anual, "%.2f");
            set_edit_double(ctx->hTxtImpuestos, ctx->pVehiculo->impuestos_anual, "%.2f");
            set_edit_double(ctx->hTxtLimpieza, ctx->pVehiculo->costo_limpieza_anual, "%.2f");
        }

        SetFocus(ctx->hTxtNombre);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == IDOK) {
            char nombre[MAX_NAME_LEN];
            GetWindowTextA(ctx->hTxtNombre, nombre, sizeof(nombre));

            // Quitar espacios adelante
            char* p = nombre;
            while (*p == ' ') p++;

            if (strlen(p) == 0) {
                MessageBoxA(hWnd, "Por favor ingrese un nombre o modelo para el vehículo.",
                    "Campo requerido", MB_ICONWARNING | MB_OK);
                SetFocus(ctx->hTxtNombre);
                return 0;
            }

            char buf[64];
            snprintf(ctx->pVehiculo->nombre, MAX_NAME_LEN, "%s", p);

            GetWindowTextA(ctx->hTxtPrecioComb, buf, sizeof(buf));
            ctx->pVehiculo->precio_combustible = parse_double(buf);

            GetWindowTextA(ctx->hTxtConsCiudad, buf, sizeof(buf));
            ctx->pVehiculo->consumo_ciudad = parse_double(buf);

            GetWindowTextA(ctx->hTxtConsCarretera, buf, sizeof(buf));
            ctx->pVehiculo->consumo_carretera = parse_double(buf);

            GetWindowTextA(ctx->hTxtNeumaticos, buf, sizeof(buf));
            ctx->pVehiculo->costo_neumaticos_km = parse_double(buf);

            GetWindowTextA(ctx->hTxtMantenimiento, buf, sizeof(buf));
            ctx->pVehiculo->costo_mant_km = parse_double(buf);

            GetWindowTextA(ctx->hTxtValorInicial, buf, sizeof(buf));
            ctx->pVehiculo->valor_inicial = parse_double(buf);

            GetWindowTextA(ctx->hTxtValorFinal, buf, sizeof(buf));
            ctx->pVehiculo->valor_final = parse_double(buf);

            GetWindowTextA(ctx->hTxtTiempoAnios, buf, sizeof(buf));
            ctx->pVehiculo->tiempo_uso_anios = parse_double(buf);
            if (ctx->pVehiculo->tiempo_uso_anios <= 0) ctx->pVehiculo->tiempo_uso_anios = 1.0;

            GetWindowTextA(ctx->hTxtKmAnuales, buf, sizeof(buf));
            ctx->pVehiculo->km_anuales = parse_double(buf);
            if (ctx->pVehiculo->km_anuales <= 0) ctx->pVehiculo->km_anuales = 15000.0;

            GetWindowTextA(ctx->hTxtSeguro, buf, sizeof(buf));
            ctx->pVehiculo->seguro_anual = parse_double(buf);

            GetWindowTextA(ctx->hTxtImpuestos, buf, sizeof(buf));
            ctx->pVehiculo->impuestos_anual = parse_double(buf);

            GetWindowTextA(ctx->hTxtLimpieza, buf, sizeof(buf));
            ctx->pVehiculo->costo_limpieza_anual = parse_double(buf);

            ctx->guardado = 1;
            DestroyWindow(hWnd);
            return 0;
        }
        else if (id == IDCANCEL) {
            ctx->guardado = 0;
            DestroyWindow(hWnd);
            return 0;
        }
        break;
    }

    case WM_CLOSE:
        ctx->guardado = 0;
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        if (ctx && ctx->hFont) {
            DeleteObject(ctx->hFont);
        }
        return 0;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

int MostrarDialogoVehiculo(HWND hwndParent, Vehiculo* vehiculo, int es_edicion) {
    static int class_registered = 0;
    HINSTANCE hInst = GetModuleHandle(NULL);

    if (!class_registered) {
        WNDCLASSEXA wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEXA);
        wc.lpfnWndProc = VehicleDlgProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = "CarCostVehicleDlgClass";
        RegisterClassExA(&wc);
        class_registered = 1;
    }

    DlgContext ctx;
    memset(&ctx, 0, sizeof(DlgContext));
    ctx.pVehiculo = vehiculo;
    ctx.es_edicion = es_edicion;
    ctx.guardado = 0;

    const char* title = es_edicion ? "Editar Vehículo" : "Nuevo Vehículo";

    // Calcular posición centrada respecto al padre
    RECT rcParent;
    GetWindowRect(hwndParent, &rcParent);
    int dlgW = 530;
    int dlgH = 635;
    int x = rcParent.left + (rcParent.right - rcParent.left - dlgW) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - dlgH) / 2;
    if (x < 10) x = 10;
    if (y < 10) y = 10;

    HWND hDlg = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "CarCostVehicleDlgClass",
        title,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, dlgW, dlgH,
        hwndParent,
        NULL,
        hInst,
        &ctx);

    if (!hDlg) return 0;

    EnableWindow(hwndParent, FALSE);

    MSG msg;
    while (IsWindow(hDlg) && GetMessageA(&msg, NULL, 0, 0)) {
        if (!IsDialogMessageA(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    EnableWindow(hwndParent, TRUE);
    SetActiveWindow(hwndParent);

    return ctx.guardado;
}
