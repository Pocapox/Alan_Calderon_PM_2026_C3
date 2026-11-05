#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vehicle.h"
#include "dlg_vehicle.h"

// IDs de controles
#define IDC_LST_VEHICLES       1001
#define IDC_BTN_NUEVO          1002
#define IDC_BTN_EDITAR         1003
#define IDC_BTN_ELIMINAR       1004
#define IDC_TXT_FICHA          1005

#define IDC_EDT_DISTANCIA      1010
#define IDC_EDT_PRECIO_VIAJE   1011
#define IDC_EDT_PCT_CARRETERA  1012
#define IDC_EDT_PCT_CIUDAD     1013
#define IDC_TRK_CARRETERA      1014
#define IDC_BTN_CALCULAR       1015

#define IDC_LST_RESULTADOS     1020
#define IDC_BTN_COPIAR         1021
#define IDC_BTN_GUARDAR        1022
#define IDC_LBL_RES_TOTAL      1023
#define IDC_LBL_RES_POR_KM     1024
#define IDC_LBL_RES_EXTRA      1025

// Estado de la aplicación
static Vehiculo g_vehiculos[MAX_VEHICLES];
static int g_total_vehiculos = 0;
static int g_vehiculo_sel = -1;
static ResultadoViaje g_ultimo_resultado;
static ParametrosViaje g_ultimo_viaje;
static int g_hay_resultado = 0;
static int g_syncing_pct = 1; // Inicia en 1 para evitar eventos durante WM_CREATE

// Fuentes
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontLargeBold = NULL;
static HFONT g_hFontMono = NULL;

// Handlers de controles
static HWND g_hLstVehicles;
static HWND g_hTxtFicha;
static HWND g_hEdtDistancia;
static HWND g_hEdtPrecioViaje;
static HWND g_hEdtPctCarretera;
static HWND g_hEdtPctCiudad;
static HWND g_hTrkCarretera;
static HWND g_hLstResultados;
static HWND g_hLblResTotal;
static HWND g_hLblResPorKm;
static HWND g_hLblResExtra;

// Prototipos
static void ActualizarListaVehiculos(HWND hWnd);
static void ActualizarFichaTecnica(void);
static void EjecutarCalculo(HWND hWnd, int mostrar_errores);
static void CopiarResultadosAlPortapapeles(HWND hWnd);
static double ParseDouble(const char* str);

static double ParseDouble(const char* str) {
    char buf[64];
    strncpy(buf, str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    for (int i = 0; buf[i]; i++) {
        if (buf[i] == ',') buf[i] = '.';
    }
    return atof(buf);
}

static void InicializarFuentes(void) {
    g_hFontNormal = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    g_hFontBold = CreateFontA(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    g_hFontLargeBold = CreateFontA(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    g_hFontMono = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
}

static void LiberarFuentes(void) {
    if (g_hFontNormal) DeleteObject(g_hFontNormal);
    if (g_hFontBold) DeleteObject(g_hFontBold);
    if (g_hFontLargeBold) DeleteObject(g_hFontLargeBold);
    if (g_hFontMono) DeleteObject(g_hFontMono);
}

static void ConfigurarColumnasResultados(HWND hList) {
    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMNA col;
    memset(&col, 0, sizeof(LVCOLUMNA));
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;

    col.pszText = "Concepto de Costo";
    col.cx = 150;
    col.fmt = LVCFMT_LEFT;
    col.iSubItem = 0;
    ListView_InsertColumn(hList, 0, &col);

    col.pszText = "Costo Viaje ($)";
    col.cx = 110;
    col.fmt = LVCFMT_RIGHT;
    col.iSubItem = 1;
    ListView_InsertColumn(hList, 1, &col);

    col.pszText = "Costo por KM ($/km)";
    col.cx = 135;
    col.fmt = LVCFMT_RIGHT;
    col.iSubItem = 2;
    ListView_InsertColumn(hList, 2, &col);

    col.pszText = "% del Total";
    col.cx = 85;
    col.fmt = LVCFMT_RIGHT;
    col.iSubItem = 3;
    ListView_InsertColumn(hList, 3, &col);

    col.pszText = "Detalle y Parámetros";
    col.cx = 180;
    col.fmt = LVCFMT_LEFT;
    col.iSubItem = 4;
    ListView_InsertColumn(hList, 4, &col);
}

static void AgregarFilaResultado(HWND hList, int idx, const char* concepto, double viaje, double por_km, double total_viaje, const char* detalle) {
    LVITEMA item;
    memset(&item, 0, sizeof(LVITEMA));
    item.mask = LVIF_TEXT;
    item.iItem = idx;
    item.iSubItem = 0;
    item.pszText = (char*)concepto;
    ListView_InsertItem(hList, &item);

    char buf[128];
    snprintf(buf, sizeof(buf), "$ %.2f", viaje);
    ListView_SetItemText(hList, idx, 1, buf);

    snprintf(buf, sizeof(buf), "$ %.4f", por_km);
    ListView_SetItemText(hList, idx, 2, buf);

    double pct = (total_viaje > 0.0001) ? (viaje * 100.0 / total_viaje) : 0.0;
    snprintf(buf, sizeof(buf), "%.1f %%", pct);
    ListView_SetItemText(hList, idx, 3, buf);

    ListView_SetItemText(hList, idx, 4, (char*)detalle);
}

static void ActualizarListaVehiculos(HWND hWnd) {
    SendMessage(g_hLstVehicles, LB_RESETCONTENT, 0, 0);

    for (int i = 0; i < g_total_vehiculos; i++) {
        SendMessageA(g_hLstVehicles, LB_ADDSTRING, 0, (LPARAM)g_vehiculos[i].nombre);
    }

    if (g_total_vehiculos > 0) {
        if (g_vehiculo_sel < 0 || g_vehiculo_sel >= g_total_vehiculos) {
            g_vehiculo_sel = 0;
        }
        SendMessage(g_hLstVehicles, LB_SETCURSEL, (WPARAM)g_vehiculo_sel, 0);
        ActualizarFichaTecnica();

        // Actualizar precio de combustible sugerido
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", g_vehiculos[g_vehiculo_sel].precio_combustible);
        SetWindowTextA(g_hEdtPrecioViaje, buf);
    } else {
        g_vehiculo_sel = -1;
        SetWindowTextA(g_hTxtFicha, "No hay vehículos registrados.\r\nHaga clic en '+ Nuevo' para agregar uno.");
        SetWindowTextA(g_hLblResTotal, "$ 0.00");
        SetWindowTextA(g_hLblResPorKm, "$ 0.0000 / km");
        SetWindowTextA(g_hLblResExtra, "No hay vehiculos disponibles para calcular.");
        InvalidateRect(g_hLblResTotal, NULL, TRUE);
        InvalidateRect(g_hLblResPorKm, NULL, TRUE);
        InvalidateRect(g_hLblResExtra, NULL, TRUE);
        ListView_DeleteAllItems(g_hLstResultados);
    }
}

static void ActualizarFichaTecnica(void) {
    if (g_vehiculo_sel < 0 || g_vehiculo_sel >= g_total_vehiculos) {
        SetWindowTextA(g_hTxtFicha, "Seleccione un vehículo para ver su ficha técnica.");
        return;
    }

    const Vehiculo* v = &g_vehiculos[g_vehiculo_sel];
    double km_anual = (v->km_anuales > 0) ? v->km_anuales : 15000.0;
    double dep_anual = (v->tiempo_uso_anios > 0) ? ((v->valor_inicial - v->valor_final) / v->tiempo_uso_anios) : 0.0;
    if (dep_anual < 0) dep_anual = 0;
    double dep_km = dep_anual / km_anual;
    double seg_km = v->seguro_anual / km_anual;
    double imp_km = v->impuestos_anual / km_anual;
    double limp_km = v->costo_limpieza_anual / km_anual;
    double costo_fijo_km = dep_km + seg_km + imp_km + limp_km;

    char ficha[1024];
    snprintf(ficha, sizeof(ficha),
        "VEHICULO: %s\r\n"
        "====================================\r\n"
        "1. Combustible:\r\n"
        "   - Ciudad:        %.2f L/100 km\r\n"
        "   - Carretera:     %.2f L/100 km\r\n"
        "   - Precio base:   $ %.3f / Litro\r\n\r\n"
        "2. Costos directos por KM:\r\n"
        "   - Neumaticos:    $ %.4f / km\r\n"
        "   - Mantenimiento: $ %.4f / km\r\n\r\n"
        "3. Depreciacion:\r\n"
        "   - Valor inicial: $ %.2f\r\n"
        "   - Valor residual:$ %.2f\r\n"
        "   - Uso estimado:  %.1f anios\r\n"
        "   - Km anuales:    %.0f km/anio\r\n"
        "   -> Costo Deprec: $ %.4f / km\r\n\r\n"
        "4. Costos fijos anuales:\r\n"
        "   - Seguro:        $ %.2f / anio ($%.4f/km)\r\n"
        "   - Impuestos:     $ %.2f / anio ($%.4f/km)\r\n"
        "   - Limpieza:      $ %.2f / anio ($%.4f/km)\r\n"
        "====================================\r\n"
        "Costo Fijo Base:    $ %.4f / km\r\n",
        v->nombre,
        v->consumo_ciudad,
        v->consumo_carretera,
        v->precio_combustible,
        v->costo_neumaticos_km,
        v->costo_mant_km,
        v->valor_inicial,
        v->valor_final,
        v->tiempo_uso_anios,
        km_anual,
        dep_km,
        v->seguro_anual, seg_km,
        v->impuestos_anual, imp_km,
        v->costo_limpieza_anual, limp_km,
        costo_fijo_km);

    SetWindowTextA(g_hTxtFicha, ficha);
    InvalidateRect(g_hTxtFicha, NULL, TRUE);
}

static void EjecutarCalculo(HWND hWnd, int mostrar_errores) {
    if (g_vehiculo_sel < 0 || g_vehiculo_sel >= g_total_vehiculos) {
        if (mostrar_errores) {
            MessageBoxA(hWnd, "Debe seleccionar un vehículo de la lista para calcular.",
                "Vehículo no seleccionado", MB_ICONWARNING | MB_OK);
        }
        return;
    }

    char bufDist[64], bufPrecio[64], bufPctCarr[64];
    GetWindowTextA(g_hEdtDistancia, bufDist, sizeof(bufDist));
    GetWindowTextA(g_hEdtPrecioViaje, bufPrecio, sizeof(bufPrecio));
    GetWindowTextA(g_hEdtPctCarretera, bufPctCarr, sizeof(bufPctCarr));

    double dist = ParseDouble(bufDist);
    if (dist <= 0.0) {
        if (mostrar_errores) {
            MessageBoxA(hWnd, "La distancia del viaje debe ser mayor a 0 km.",
                "Distancia inválida", MB_ICONWARNING | MB_OK);
            SetFocus(g_hEdtDistancia);
        }
        return;
    }

    double precio = ParseDouble(bufPrecio);
    if (precio <= 0.0) {
        precio = g_vehiculos[g_vehiculo_sel].precio_combustible;
    }

    double pct_carr = ParseDouble(bufPctCarr);
    if (pct_carr < 0.0) pct_carr = 0.0;
    if (pct_carr > 100.0) pct_carr = 100.0;
    double pct_ciud = 100.0 - pct_carr;

    ParametrosViaje viaje;
    viaje.distancia_km = dist;
    viaje.pct_carretera = pct_carr;
    viaje.pct_ciudad = pct_ciud;
    viaje.precio_combustible = precio;

    ResultadoViaje res;
    if (!calcular_costo_viaje(&g_vehiculos[g_vehiculo_sel], &viaje, &res)) {
        if (mostrar_errores) {
            MessageBoxA(hWnd, "Error al procesar el cálculo del viaje.", "Error", MB_ICONERROR | MB_OK);
        }
        return;
    }

    g_ultimo_resultado = res;
    g_ultimo_viaje = viaje;
    g_hay_resultado = 1;

    // Actualizar Banners de Resultados con borrado garantizado del texto anterior
    char bannerTotal[64], bannerPorKm[64];
    snprintf(bannerTotal, sizeof(bannerTotal), "$ %.2f", res.costo_total);
    SetWindowTextA(g_hLblResTotal, bannerTotal);
    InvalidateRect(g_hLblResTotal, NULL, TRUE);

    snprintf(bannerPorKm, sizeof(bannerPorKm), "$ %.4f / km", res.costo_por_km);
    SetWindowTextA(g_hLblResPorKm, bannerPorKm);
    InvalidateRect(g_hLblResPorKm, NULL, TRUE);

    char bannerExtra[256];
    snprintf(bannerExtra, sizeof(bannerExtra),
        "Distancia: %.1f km  |  Consumo Ponderado: %.2f L/100km  |  Combustible: %.2f L  |  Precio: $%.2f/L",
        dist, res.consumo_mixto_l100km, res.litros_consumidos, precio);
    SetWindowTextA(g_hLblResExtra, bannerExtra);
    InvalidateRect(g_hLblResExtra, NULL, TRUE);

    // Llenar ListView
    SendMessage(g_hLstResultados, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(g_hLstResultados);

    char det[128];
    // 1. Combustible
    snprintf(det, sizeof(det), "%.2f L consumidos (%.1f%% carr / %.1f%% ciud)",
        res.litros_consumidos, pct_carr, pct_ciud);
    AgregarFilaResultado(g_hLstResultados, 0, "1. Combustible",
        res.costo_combustible, res.km_combustible, res.costo_total, det);

    // 2. Neumáticos
    snprintf(det, sizeof(det), "Tarifa fija: $%.4f/km", res.km_neumaticos);
    AgregarFilaResultado(g_hLstResultados, 1, "2. Neumaticos",
        res.costo_neumaticos, res.km_neumaticos, res.costo_total, det);

    // 3. Mantenimiento
    snprintf(det, sizeof(det), "Tarifa fija: $%.4f/km", res.km_mantenimiento);
    AgregarFilaResultado(g_hLstResultados, 2, "3. Mantenimiento",
        res.costo_mantenimiento, res.km_mantenimiento, res.costo_total, det);

    // 4. Depreciación
    snprintf(det, sizeof(det), "Prorrateado (%.0f km/anio)", g_vehiculos[g_vehiculo_sel].km_anuales);
    AgregarFilaResultado(g_hLstResultados, 3, "4. Depreciacion",
        res.costo_depreciacion, res.km_depreciacion, res.costo_total, det);

    // 5. Seguro
    snprintf(det, sizeof(det), "Prorrateado ($%.0f/anio)", g_vehiculos[g_vehiculo_sel].seguro_anual);
    AgregarFilaResultado(g_hLstResultados, 4, "5. Seguro Anual",
        res.costo_seguro, res.km_seguro, res.costo_total, det);

    // 6. Impuestos
    snprintf(det, sizeof(det), "Prorrateado ($%.0f/anio)", g_vehiculos[g_vehiculo_sel].impuestos_anual);
    AgregarFilaResultado(g_hLstResultados, 5, "6. Impuestos / Patente",
        res.costo_impuestos, res.km_impuestos, res.costo_total, det);

    // 7. Limpieza
    snprintf(det, sizeof(det), "Prorrateado ($%.0f/anio)", g_vehiculos[g_vehiculo_sel].costo_limpieza_anual);
    AgregarFilaResultado(g_hLstResultados, 6, "7. Costo Limpieza",
        res.costo_limpieza, res.km_limpieza, res.costo_total, det);

    // 8. TOTAL
    snprintf(det, sizeof(det), "Suma de los 7 parametros de costo");
    AgregarFilaResultado(g_hLstResultados, 7, "TOTAL GENERAL",
        res.costo_total, res.costo_por_km, res.costo_total, det);

    SendMessage(g_hLstResultados, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(g_hLstResultados, NULL, TRUE);
}

static void CopiarResultadosAlPortapapeles(HWND hWnd) {
    if (!g_hay_resultado || g_vehiculo_sel < 0) {
        MessageBoxA(hWnd, "Primero debe realizar un cálculo para copiar los resultados.",
            "Sin resultados", MB_ICONINFORMATION | MB_OK);
        return;
    }

    const Vehiculo* v = &g_vehiculos[g_vehiculo_sel];
    const ResultadoViaje* r = &g_ultimo_resultado;
    const ParametrosViaje* p = &g_ultimo_viaje;

    char buffer[2048];
    snprintf(buffer, sizeof(buffer),
        "==========================================================\r\n"
        "      REPORTE DE COSTO DE VIAJE POR KILOMETRO (C)\r\n"
        "==========================================================\r\n"
        "Vehiculo:                %s\r\n"
        "Distancia Total:         %.2f km\r\n"
        "Conduccion Carretera:    %.1f %%\r\n"
        "Conduccion Ciudad:       %.1f %%\r\n"
        "Consumo Ponderado:       %.2f L/100 km\r\n"
        "Litros de Combustible:   %.2f L (a $%.2f/L)\r\n"
        "----------------------------------------------------------\r\n"
        "DESGLOSE DE COSTOS:\r\n"
        "1. Combustible:      $ %8.2f  ($ %.4f/km) [%5.1f%%]\r\n"
        "2. Neumaticos:       $ %8.2f  ($ %.4f/km) [%5.1f%%]\r\n"
        "3. Mantenimiento:    $ %8.2f  ($ %.4f/km) [%5.1f%%]\r\n"
        "4. Depreciacion:     $ %8.2f  ($ %.4f/km) [%5.1f%%]\r\n"
        "5. Seguro Anual:     $ %8.2f  ($ %.4f/km) [%5.1f%%]\r\n"
        "6. Impuestos:        $ %8.2f  ($ %.4f/km) [%5.1f%%]\r\n"
        "7. Limpieza:         $ %8.2f  ($ %.4f/km) [%5.1f%%]\r\n"
        "----------------------------------------------------------\r\n"
        "COSTO TOTAL VIAJE:   $ %8.2f\r\n"
        "COSTO POR KM:        $ %8.4f / km\r\n"
        "==========================================================\r\n",
        v->nombre,
        p->distancia_km,
        p->pct_carretera,
        p->pct_ciudad,
        r->consumo_mixto_l100km,
        r->litros_consumidos,
        p->precio_combustible,
        r->costo_combustible, r->km_combustible, (r->costo_combustible * 100.0 / r->costo_total),
        r->costo_neumaticos, r->km_neumaticos, (r->costo_neumaticos * 100.0 / r->costo_total),
        r->costo_mantenimiento, r->km_mantenimiento, (r->costo_mantenimiento * 100.0 / r->costo_total),
        r->costo_depreciacion, r->km_depreciacion, (r->costo_depreciacion * 100.0 / r->costo_total),
        r->costo_seguro, r->km_seguro, (r->costo_seguro * 100.0 / r->costo_total),
        r->costo_impuestos, r->km_impuestos, (r->costo_impuestos * 100.0 / r->costo_total),
        r->costo_limpieza, r->km_limpieza, (r->costo_limpieza * 100.0 / r->costo_total),
        r->costo_total,
        r->costo_por_km
    );

    if (OpenClipboard(hWnd)) {
        EmptyClipboard();
        size_t len = strlen(buffer) + 1;
        HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len);
        if (hGlob) {
            memcpy(GlobalLock(hGlob), buffer, len);
            GlobalUnlock(hGlob);
            SetClipboardData(CF_TEXT, hGlob);
        }
        CloseClipboard();
        MessageBoxA(hWnd, "El reporte completo ha sido copiado al portapapeles exitosamente.",
            "Copiado", MB_ICONINFORMATION | MB_OK);
    }
}

static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInst = ((CREATESTRUCT*)lParam)->hInstance;

        // Desactivar sincronización de eventos durante la construcción de la UI
        g_syncing_pct = 1;

        // ==========================================
        // 1. PANEL IZQUIERDO: GESTION DE VEHICULOS
        // ==========================================
        HWND hGrpVeh = CreateWindowExA(0, "BUTTON", " Gestion de Vehiculos ",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            12, 10, 310, 645, hWnd, NULL, hInst, NULL);
        SendMessage(hGrpVeh, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        // Lista de vehículos
        g_hLstVehicles = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_TABSTOP,
            24, 35, 286, 130, hWnd, (HMENU)IDC_LST_VEHICLES, hInst, NULL);
        SendMessage(g_hLstVehicles, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        // Botones Nuevo, Editar, Eliminar
        HWND hBtnNuevo = CreateWindowExA(0, "BUTTON", "+ Nuevo",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            24, 172, 90, 28, hWnd, (HMENU)IDC_BTN_NUEVO, hInst, NULL);
        SendMessage(hBtnNuevo, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        HWND hBtnEditar = CreateWindowExA(0, "BUTTON", "Editar",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            122, 172, 90, 28, hWnd, (HMENU)IDC_BTN_EDITAR, hInst, NULL);
        SendMessage(hBtnEditar, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        HWND hBtnEliminar = CreateWindowExA(0, "BUTTON", "Eliminar",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            220, 172, 90, 28, hWnd, (HMENU)IDC_BTN_ELIMINAR, hInst, NULL);
        SendMessage(hBtnEliminar, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        // Ficha técnica
        HWND hLblFicha = CreateWindowExA(0, "STATIC", "Ficha Tecnica y Parametros:",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            24, 212, 286, 18, hWnd, NULL, hInst, NULL);
        SendMessage(hLblFicha, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hTxtFicha = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
            24, 234, 286, 405, hWnd, (HMENU)IDC_TXT_FICHA, hInst, NULL);
        SendMessage(g_hTxtFicha, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        // ==========================================
        // 2. PANEL DERECHO: PARAMETROS DEL VIAJE
        // ==========================================
        HWND hGrpViaje = CreateWindowExA(0, "BUTTON", " Parametros del Viaje ",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            332, 10, 680, 160, hWnd, NULL, hInst, NULL);
        SendMessage(hGrpViaje, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        // Distancia Total
        HWND hLblDist = CreateWindowExA(0, "STATIC", "Distancia Total (km):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 350, 36, 150, 20, hWnd, NULL, hInst, NULL);
        SendMessage(hLblDist, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        g_hEdtDistancia = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "150.0",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 505, 34, 85, 24, hWnd, (HMENU)IDC_EDT_DISTANCIA, hInst, NULL);
        SendMessage(g_hEdtDistancia, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        // Precio Combustible Viaje
        HWND hLblPrec = CreateWindowExA(0, "STATIC", "Precio Combustible ($/L):",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 630, 36, 175, 20, hWnd, NULL, hInst, NULL);
        SendMessage(hLblPrec, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        g_hEdtPrecioViaje = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1.25",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 810, 34, 85, 24, hWnd, (HMENU)IDC_EDT_PRECIO_VIAJE, hInst, NULL);
        SendMessage(g_hEdtPrecioViaje, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        // Porcentajes Carretera vs Ciudad
        HWND hLblCarr = CreateWindowExA(0, "STATIC", "% en Carretera:",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 350, 72, 105, 20, hWnd, NULL, hInst, NULL);
        SendMessage(hLblCarr, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        g_hEdtPctCarretera = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "70.0",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 460, 70, 50, 24, hWnd, (HMENU)IDC_EDT_PCT_CARRETERA, hInst, NULL);
        SendMessage(g_hEdtPctCarretera, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        // Trackbar / Slider
        g_hTrkCarretera = CreateWindowExA(0, TRACKBAR_CLASSA, "Trackbar Carretera",
            WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_NOTICKS | WS_TABSTOP,
            520, 68, 220, 28, hWnd, (HMENU)IDC_TRK_CARRETERA, hInst, NULL);
        SendMessage(g_hTrkCarretera, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        SendMessage(g_hTrkCarretera, TBM_SETPOS, TRUE, 70);

        HWND hLblCiud = CreateWindowExA(0, "STATIC", "% en Ciudad:",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 755, 72, 90, 20, hWnd, NULL, hInst, NULL);
        SendMessage(hLblCiud, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        g_hEdtPctCiudad = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "30.0",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 850, 70, 50, 24, hWnd, (HMENU)IDC_EDT_PCT_CIUDAD, hInst, NULL);
        SendMessage(g_hEdtPctCiudad, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        // Boton Calcular
        HWND hBtnCalc = CreateWindowExA(0, "BUTTON", "CALCULAR COSTO DEL VIAJE",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            350, 110, 645, 42, hWnd, (HMENU)IDC_BTN_CALCULAR, hInst, NULL);
        SendMessage(hBtnCalc, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        // ==========================================
        // 3. PANEL DERECHO: RESULTADOS Y DESGLOSE
        // ==========================================
        HWND hGrpRes = CreateWindowExA(0, "BUTTON", " Resultados del Viaje ",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            332, 178, 680, 477, hWnd, NULL, hInst, NULL);
        SendMessage(hGrpRes, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        // Tarjetas destacadas de resultado (Readouts independientes con WS_EX_CLIENTEDGE)
        HWND hLblTitTotal = CreateWindowExA(0, "STATIC", "COSTO TOTAL DEL VIAJE",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 350, 202, 310, 16, hWnd, NULL, hInst, NULL);
        SendMessage(hLblTitTotal, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hLblResTotal = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "$ 0.00",
            WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
            350, 222, 310, 46, hWnd, (HMENU)IDC_LBL_RES_TOTAL, hInst, NULL);
        SendMessage(g_hLblResTotal, WM_SETFONT, (WPARAM)g_hFontLargeBold, TRUE);

        HWND hLblTitPorKm = CreateWindowExA(0, "STATIC", "COSTO POR KILOMETRO",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 685, 202, 310, 16, hWnd, NULL, hInst, NULL);
        SendMessage(hLblTitPorKm, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

        g_hLblResPorKm = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "$ 0.0000 / km",
            WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
            685, 222, 310, 46, hWnd, (HMENU)IDC_LBL_RES_POR_KM, hInst, NULL);
        SendMessage(g_hLblResPorKm, WM_SETFONT, (WPARAM)g_hFontLargeBold, TRUE);

        // ListView para el desglose detallado de los 7 parámetros
        g_hLstResultados = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP,
            350, 280, 645, 260, hWnd, (HMENU)IDC_LST_RESULTADOS, hInst, NULL);
        SendMessage(g_hLstResultados, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        ConfigurarColumnasResultados(g_hLstResultados);

        // Barra inferior de datos adicionales (L/100km, litros totales)
        g_hLblResExtra = CreateWindowExA(WS_EX_STATICEDGE, "STATIC", "Ingrese distancia y parametros y presione 'Calcular Costo del Viaje'.",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_LEFT,
            350, 550, 645, 26, hWnd, (HMENU)IDC_LBL_RES_EXTRA, hInst, NULL);
        SendMessage(g_hLblResExtra, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        // Botones inferiores de resultados
        HWND hBtnCopiar = CreateWindowExA(0, "BUTTON", "Copiar Reporte",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            670, 610, 155, 32, hWnd, (HMENU)IDC_BTN_COPIAR, hInst, NULL);
        SendMessage(hBtnCopiar, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        HWND hBtnGuardar = CreateWindowExA(0, "BUTTON", "Guardar Vehiculos",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            840, 610, 155, 32, hWnd, (HMENU)IDC_BTN_GUARDAR, hInst, NULL);
        SendMessage(hBtnGuardar, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

        // Habilitar sincronización
        g_syncing_pct = 0;

        // Cargar vehículos desde disco o usar por defecto
        int cargados = cargar_vehiculos(g_vehiculos, MAX_VEHICLES, DEFAULT_DATA_FILE);
        if (cargados <= 0) {
            cargar_vehiculos_por_defecto(g_vehiculos, &g_total_vehiculos);
            guardar_vehiculos(g_vehiculos, g_total_vehiculos, DEFAULT_DATA_FILE);
        } else {
            g_total_vehiculos = cargados;
        }

        ActualizarListaVehiculos(hWnd);

        // Ejecutar un primer cálculo inicial de demostración (silencioso)
        EjecutarCalculo(hWnd, 0);
        return 0;
    }

    case WM_HSCROLL: {
        // Evento del Trackbar de Carretera
        if ((HWND)lParam == g_hTrkCarretera) {
            if (!g_syncing_pct) {
                g_syncing_pct = 1;
                int pos = (int)SendMessage(g_hTrkCarretera, TBM_GETPOS, 0, 0);
                char buf[32];
                snprintf(buf, sizeof(buf), "%.1f", (double)pos);
                SetWindowTextA(g_hEdtPctCarretera, buf);
                snprintf(buf, sizeof(buf), "%.1f", (double)(100 - pos));
                SetWindowTextA(g_hEdtPctCiudad, buf);
                g_syncing_pct = 0;

                // Recalcular automáticamente si hay datos válidos
                EjecutarCalculo(hWnd, 0);
            }
        }
        break;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int notif = HIWORD(wParam);

        // Cambio de selección en la lista de vehículos
        if (id == IDC_LST_VEHICLES && notif == LBN_SELCHANGE) {
            int sel = (int)SendMessage(g_hLstVehicles, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR) {
                g_vehiculo_sel = sel;
                ActualizarFichaTecnica();
                char buf[32];
                snprintf(buf, sizeof(buf), "%.2f", g_vehiculos[sel].precio_combustible);
                SetWindowTextA(g_hEdtPrecioViaje, buf);

                // Calcular automáticamente para el nuevo vehículo
                EjecutarCalculo(hWnd, 0);
            }
            return 0;
        }

        // Modificación en tiempo real de % en carretera
        if (id == IDC_EDT_PCT_CARRETERA && (notif == EN_CHANGE || notif == EN_KILLFOCUS)) {
            if (!g_syncing_pct) {
                char buf[32];
                GetWindowTextA(g_hEdtPctCarretera, buf, sizeof(buf));
                if (strlen(buf) > 0) {
                    g_syncing_pct = 1;
                    double v = ParseDouble(buf);
                    if (v < 0.0) v = 0.0;
                    if (v > 100.0) v = 100.0;
                    double ciud = 100.0 - v;
                    char bufCiud[32];
                    snprintf(bufCiud, sizeof(bufCiud), "%.1f", ciud);
                    SetWindowTextA(g_hEdtPctCiudad, bufCiud);
                    SendMessage(g_hTrkCarretera, TBM_SETPOS, TRUE, (LPARAM)(int)(v + 0.5));
                    g_syncing_pct = 0;
                }
            }
            return 0;
        }

        // Modificación en tiempo real de % en ciudad
        if (id == IDC_EDT_PCT_CIUDAD && (notif == EN_CHANGE || notif == EN_KILLFOCUS)) {
            if (!g_syncing_pct) {
                char buf[32];
                GetWindowTextA(g_hEdtPctCiudad, buf, sizeof(buf));
                if (strlen(buf) > 0) {
                    g_syncing_pct = 1;
                    double v = ParseDouble(buf);
                    if (v < 0.0) v = 0.0;
                    if (v > 100.0) v = 100.0;
                    double carr = 100.0 - v;
                    char bufCarr[32];
                    snprintf(bufCarr, sizeof(bufCarr), "%.1f", carr);
                    SetWindowTextA(g_hEdtPctCarretera, bufCarr);
                    SendMessage(g_hTrkCarretera, TBM_SETPOS, TRUE, (LPARAM)(int)(carr + 0.5));
                    g_syncing_pct = 0;
                }
            }
            return 0;
        }

        // Botón Calcular (aquí sí se muestran errores si faltan datos)
        if (id == IDC_BTN_CALCULAR) {
            EjecutarCalculo(hWnd, 1);
            return 0;
        }

        // Botón Nuevo Vehículo
        if (id == IDC_BTN_NUEVO) {
            if (g_total_vehiculos >= MAX_VEHICLES) {
                MessageBoxA(hWnd, "Se ha alcanzado el límite máximo de vehículos.", "Límite alcanzado", MB_ICONWARNING | MB_OK);
                return 0;
            }

            Vehiculo nuevo;
            memset(&nuevo, 0, sizeof(Vehiculo));
            strncpy(nuevo.nombre, "Nuevo Vehículo", MAX_NAME_LEN - 1);
            nuevo.precio_combustible = 1.25;
            nuevo.consumo_ciudad = 9.0;
            nuevo.consumo_carretera = 6.5;
            nuevo.seguro_anual = 700.0;
            nuevo.costo_neumaticos_km = 0.025;
            nuevo.costo_mant_km = 0.035;
            nuevo.valor_inicial = 20000.0;
            nuevo.tiempo_uso_anios = 5.0;
            nuevo.valor_final = 9000.0;
            nuevo.km_anuales = 15000.0;
            nuevo.impuestos_anual = 300.0;
            nuevo.costo_limpieza_anual = 200.0;

            if (MostrarDialogoVehiculo(hWnd, &nuevo, 0)) {
                g_vehiculos[g_total_vehiculos] = nuevo;
                g_vehiculo_sel = g_total_vehiculos;
                g_total_vehiculos++;
                guardar_vehiculos(g_vehiculos, g_total_vehiculos, DEFAULT_DATA_FILE);
                ActualizarListaVehiculos(hWnd);
                EjecutarCalculo(hWnd, 0);
                MessageBoxA(hWnd, "Vehículo agregado y guardado con éxito.", "Vehículo Creado", MB_ICONINFORMATION | MB_OK);
            }
            return 0;
        }

        // Botón Editar Vehículo
        if (id == IDC_BTN_EDITAR) {
            if (g_vehiculo_sel < 0 || g_vehiculo_sel >= g_total_vehiculos) {
                MessageBoxA(hWnd, "Seleccione un vehículo de la lista para editar.", "Sin selección", MB_ICONWARNING | MB_OK);
                return 0;
            }

            Vehiculo editado = g_vehiculos[g_vehiculo_sel];
            if (MostrarDialogoVehiculo(hWnd, &editado, 1)) {
                g_vehiculos[g_vehiculo_sel] = editado;
                guardar_vehiculos(g_vehiculos, g_total_vehiculos, DEFAULT_DATA_FILE);
                ActualizarListaVehiculos(hWnd);
                EjecutarCalculo(hWnd, 0);
                MessageBoxA(hWnd, "Vehículo actualizado y guardado correctamente.", "Vehículo Editado", MB_ICONINFORMATION | MB_OK);
            }
            return 0;
        }

        // Botón Eliminar Vehículo
        if (id == IDC_BTN_ELIMINAR) {
            if (g_vehiculo_sel < 0 || g_vehiculo_sel >= g_total_vehiculos) {
                MessageBoxA(hWnd, "Seleccione un vehículo de la lista para eliminar.", "Sin selección", MB_ICONWARNING | MB_OK);
                return 0;
            }

            char msg[256];
            snprintf(msg, sizeof(msg), "¿Está seguro de que desea eliminar el vehículo '%s'?",
                g_vehiculos[g_vehiculo_sel].nombre);

            if (MessageBoxA(hWnd, msg, "Confirmar eliminación", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                for (int i = g_vehiculo_sel; i < g_total_vehiculos - 1; i++) {
                    g_vehiculos[i] = g_vehiculos[i + 1];
                }
                g_total_vehiculos--;
                if (g_vehiculo_sel >= g_total_vehiculos) {
                    g_vehiculo_sel = g_total_vehiculos - 1;
                }
                guardar_vehiculos(g_vehiculos, g_total_vehiculos, DEFAULT_DATA_FILE);
                ActualizarListaVehiculos(hWnd);
                if (g_total_vehiculos > 0) {
                    EjecutarCalculo(hWnd, 0);
                } else {
                    SetWindowTextA(g_hLblResTotal, "$ 0.00");
                    SetWindowTextA(g_hLblResPorKm, "$ 0.0000 / km");
                    InvalidateRect(g_hLblResTotal, NULL, TRUE);
                    InvalidateRect(g_hLblResPorKm, NULL, TRUE);
                    ListView_DeleteAllItems(g_hLstResultados);
                }
            }
            return 0;
        }

        // Botón Copiar al Portapapeles
        if (id == IDC_BTN_COPIAR) {
            CopiarResultadosAlPortapapeles(hWnd);
            return 0;
        }

        // Botón Guardar Vehículos
        if (id == IDC_BTN_GUARDAR) {
            if (guardar_vehiculos(g_vehiculos, g_total_vehiculos, DEFAULT_DATA_FILE)) {
                MessageBoxA(hWnd, "Datos de vehículos guardados en el archivo 'vehiculos.csv'.", "Guardado exitoso", MB_ICONINFORMATION | MB_OK);
            } else {
                MessageBoxA(hWnd, "No se pudo guardar el archivo 'vehiculos.csv'.", "Error al guardar", MB_ICONERROR | MB_OK);
            }
            return 0;
        }

        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND hwndCtl = (HWND)lParam;

        // Tarjetas destacadas de resultado: fondo blanco limpio con texto azul
        if (hwndCtl == g_hLblResTotal || hwndCtl == g_hLblResPorKm) {
            SetTextColor(hdc, RGB(0, 85, 175));
            SetBkColor(hdc, RGB(255, 255, 255));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }

        // Todas las demás etiquetas estáticas y groupboxes usan el color nativo de la ventana
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }

    case WM_DESTROY:
        LiberarFuentes();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    InicializarFuentes();

    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "CarCostMainWindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Error al registrar la clase de ventana principal.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Centrar ventana principal
    int winW = 1040;
    int winH = 705;
    int scrW = GetSystemMetrics(SM_CXSCREEN);
    int scrH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (scrW - winW) / 2;
    int posY = (scrH - winH) / 2;
    if (posX < 0) posX = 0;
    if (posY < 0) posY = 0;

    HWND hWnd = CreateWindowExA(
        0,
        "CarCostMainWindowClass",
        "Calculadora de Costo de Viaje por Kilometro (Lenguaje C)",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
        posX, posY, winW, winH,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hWnd) {
        MessageBoxA(NULL, "No se pudo crear la ventana principal.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}
