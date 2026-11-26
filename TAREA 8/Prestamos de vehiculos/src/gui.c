#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui.h"
#include "linked_list.h"
#include "loan_service.h"
#include "json_store.h"
#include "report.h"

/* Control IDs */
enum {
    ID_TAB_MAIN = 100,

    /* Tab 0: Clientes */
    ID_CLI_NOMBRE = 200,
    ID_CLI_DOC,
    ID_CLI_TEL,
    ID_CLI_BTN_ADD,
    ID_CLI_SEARCH_EDIT,
    ID_CLI_BTN_SEARCH,
    ID_CLI_BTN_ALL,
    ID_CLI_LIST,
    ID_CLI_BTN_DEL,

    /* Tab 1: Vehiculos */
    ID_VEH_MARCA = 300,
    ID_VEH_MODELO,
    ID_VEH_ANIO,
    ID_VEH_BTN_ADD,
    ID_VEH_LIST,
    ID_VEH_BTN_DEL,

    /* Tab 2: Prestamos y Pagos */
    ID_PRE_CMB_CLI = 400,
    ID_PRE_CMB_VEH,
    ID_PRE_MONTO,
    ID_PRE_TASA,
    ID_PRE_PLAZO,
    ID_PRE_BTN_CREAR,
    ID_PRE_CMB_ACTIVO,
    ID_PRE_LBL_RESUMEN,
    ID_PRE_LIST_MESES,
    ID_PRE_BTN_PAGAR_MES,
    ID_PRE_BTN_TOGGLE_MES,
    ID_PRE_ABONO_MONTO,
    ID_PRE_ABONO_FECHA,
    ID_PRE_BTN_ABONO,
    ID_PRE_LIST_ABONOS,

    /* Tab 3: Reportes */
    ID_REP_CMB_PRESTAMO = 500,
    ID_REP_BTN_GENERAR,
    ID_REP_BTN_EXPORTAR,
    ID_REP_EDIT_TEXTO
};

typedef struct {
    SistemaPrestamos *sys;
    HWND hwndMain;
    HWND hTab;
    HFONT hFontUI;
    HFONT hFontMono;
    int currentTab;

    /* Grupos de controles por pestaña */
    HWND tab0Ctrls[16];
    int tab0Count;
    HWND tab1Ctrls[16];
    int tab1Count;
    HWND tab2Ctrls[32];
    int tab2Count;
    HWND tab3Ctrls[16];
    int tab3Count;
} AppState;

static AppState g_app;

static void add_tab_ctrl(HWND *arr, int *count, int maxCount, HWND ctrl) {
    if (*count < maxCount && ctrl) {
        arr[(*count)++] = ctrl;
    }
}

static HWND make_ctrl(
    const char *cls, const char *text, DWORD style,
    int x, int y, int w, int h, int id, HWND *group, int *groupCount, int maxGroup, HFONT font
) {
    HWND ctrl = CreateWindowExA(
        0, cls, text, WS_CHILD | style,
        x, y, w, h, g_app.hwndMain, (HMENU)(INT_PTR)id, GetModuleHandle(NULL), NULL
    );
    if (ctrl && font) {
        SendMessageA(ctrl, WM_SETFONT, (WPARAM)font, TRUE);
    }
    if (group && groupCount) {
        add_tab_ctrl(group, groupCount, maxGroup, ctrl);
    }
    return ctrl;
}

static HWND make_edit(
    const char *text, DWORD extraStyle,
    int x, int y, int w, int h, int id, HWND *group, int *groupCount, int maxGroup
) {
    HWND ctrl = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", text, WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | extraStyle,
        x, y, w, h, g_app.hwndMain, (HMENU)(INT_PTR)id, GetModuleHandle(NULL), NULL
    );
    if (ctrl) {
        SendMessageA(ctrl, WM_SETFONT, (WPARAM)g_app.hFontUI, TRUE);
    }
    if (group && groupCount) {
        add_tab_ctrl(group, groupCount, maxGroup, ctrl);
    }
    return ctrl;
}

static HWND make_listview(int x, int y, int w, int h, int id, HWND *group, int *groupCount, int maxGroup) {
    HWND lv = CreateWindowExA(
        WS_EX_CLIENTEDGE, WC_LISTVIEWA, "",
        WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        x, y, w, h, g_app.hwndMain, (HMENU)(INT_PTR)id, GetModuleHandle(NULL), NULL
    );
    if (lv) {
        ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        SendMessageA(lv, WM_SETFONT, (WPARAM)g_app.hFontUI, TRUE);
    }
    if (group && groupCount) {
        add_tab_ctrl(group, groupCount, maxGroup, lv);
    }
    return lv;
}

static void lv_add_col(HWND lv, int index, const char *title, int width) {
    LVCOLUMNA col;
    memset(&col, 0, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    col.pszText = (LPSTR)title;
    col.cx = width;
    col.iSubItem = index;
    ListView_InsertColumn(lv, index, &col);
}

static void guardar_datos(void) {
    char err[256];
    if (!json_guardar(g_app.sys, DATA_JSON_PATH, err, sizeof(err))) {
        MessageBoxA(g_app.hwndMain, err, "Error al guardar JSON", MB_OK | MB_ICONERROR);
    }
}

static int get_selected_lv_id(HWND lv) {
    int idx = ListView_GetNextItem(lv, -1, LVNI_SELECTED);
    if (idx < 0) return -1;
    char buf[32] = {0};
    ListView_GetItemText(lv, idx, 0, buf, sizeof(buf));
    return atoi(buf);
}

static int get_combo_selected_item_data(HWND cmb) {
    int idx = (int)SendMessageA(cmb, CB_GETCURSEL, 0, 0);
    if (idx == CB_ERR) return -1;
    return (int)SendMessageA(cmb, CB_GETITEMDATA, (WPARAM)idx, 0);
}

static void refresh_clientes_list(const char *criterio) {
    HWND lv = GetDlgItem(g_app.hwndMain, ID_CLI_LIST);
    ListView_DeleteAllItems(lv);

    int row = 0;
    for (ClienteNode *c = g_app.sys->clientes; c; c = c->next) {
        if (criterio && criterio[0] != '\0' && !cliente_coincide_criterio(c, criterio)) {
            continue;
        }
        char idBuf[32];
        snprintf(idBuf, sizeof(idBuf), "%d", c->id);

        LVITEMA item;
        memset(&item, 0, sizeof(item));
        item.mask = LVIF_TEXT;
        item.iItem = row;
        item.pszText = idBuf;
        ListView_InsertItem(lv, &item);

        ListView_SetItemText(lv, row, 1, c->nombre);
        ListView_SetItemText(lv, row, 2, c->documento);
        ListView_SetItemText(lv, row, 3, c->telefono);
        row++;
    }
}

static void refresh_vehiculos_list(void) {
    HWND lv = GetDlgItem(g_app.hwndMain, ID_VEH_LIST);
    ListView_DeleteAllItems(lv);

    int row = 0;
    for (VehiculoNode *v = g_app.sys->vehiculos; v; v = v->next) {
        char idBuf[32], anioBuf[32];
        snprintf(idBuf, sizeof(idBuf), "%d", v->id);
        snprintf(anioBuf, sizeof(anioBuf), "%d", v->anio);

        LVITEMA item;
        memset(&item, 0, sizeof(item));
        item.mask = LVIF_TEXT;
        item.iItem = row;
        item.pszText = idBuf;
        ListView_InsertItem(lv, &item);

        ListView_SetItemText(lv, row, 1, v->marca);
        ListView_SetItemText(lv, row, 2, v->modelo);
        ListView_SetItemText(lv, row, 3, anioBuf);
        row++;
    }
}

static void refresh_prestamo_detalle(void) {
    HWND cmbActivo = GetDlgItem(g_app.hwndMain, ID_PRE_CMB_ACTIVO);
    HWND lblResumen = GetDlgItem(g_app.hwndMain, ID_PRE_LBL_RESUMEN);
    HWND lvMeses = GetDlgItem(g_app.hwndMain, ID_PRE_LIST_MESES);
    HWND lvAbonos = GetDlgItem(g_app.hwndMain, ID_PRE_LIST_ABONOS);

    ListView_DeleteAllItems(lvMeses);
    ListView_DeleteAllItems(lvAbonos);

    int prestamoId = get_combo_selected_item_data(cmbActivo);
    PrestamoNode *p = prestamo_buscar_id(g_app.sys, prestamoId);
    if (!p) {
        SetWindowTextA(lblResumen, "Seleccione o cree un prestamo para visualizar su saldo y cronograma de meses.");
        return;
    }

    int mesesPagados = 0;
    double totalCuotas = 0.0, totalAbonos = 0.0;
    prestamo_obtener_resumen(p, &mesesPagados, &totalCuotas, &totalAbonos);

    char resumen[512];
    snprintf(resumen, sizeof(resumen),
             "Prestamo #%d | Estado: %s | Monto Financiado: $%.2f | Tasa Anual: %.2f%% | Cuota Mensual: $%.2f\r\n"
             "Meses Pagados: %d / %d | Pagado en Cuotas: $%.2f | Total Abonado: $%.2f | SALDO PENDIENTE: $%.2f",
             p->id,
             p->liquidado ? "LIQUIDADO" : "ACTIVO",
             p->montoFinanciado,
             p->tasaAnual,
             p->cuotaMensual,
             mesesPagados, p->plazoMeses,
             totalCuotas,
             totalAbonos,
             p->saldoPendiente);
    SetWindowTextA(lblResumen, resumen);

    int row = 0;
    for (MesPagoNode *m = p->meses; m; m = m->next) {
        char mesBuf[16], cuotaBuf[32], capBuf[32], intBuf[32];
        snprintf(mesBuf, sizeof(mesBuf), "%d", m->numeroMes);
        snprintf(cuotaBuf, sizeof(cuotaBuf), "$%.2f", m->montoCuota);
        snprintf(capBuf, sizeof(capBuf), "$%.2f", m->capitalCuota);
        snprintf(intBuf, sizeof(intBuf), "$%.2f", m->interesCuota);

        LVITEMA item;
        memset(&item, 0, sizeof(item));
        item.mask = LVIF_TEXT;
        item.iItem = row;
        item.pszText = mesBuf;
        ListView_InsertItem(lvMeses, &item);

        ListView_SetItemText(lvMeses, row, 1, cuotaBuf);
        ListView_SetItemText(lvMeses, row, 2, capBuf);
        ListView_SetItemText(lvMeses, row, 3, intBuf);
        ListView_SetItemText(lvMeses, row, 4, m->pagado ? "PAGADO" : "NO PAGADO");
        ListView_SetItemText(lvMeses, row, 5, (m->pagado && m->fechaPago && m->fechaPago[0]) ? m->fechaPago : "-");
        row++;
    }

    int aRow = 0;
    for (AbonoNode *a = p->abonos; a; a = a->next) {
        char idBuf[16], montoBuf[32];
        snprintf(idBuf, sizeof(idBuf), "%d", a->id);
        snprintf(montoBuf, sizeof(montoBuf), "$%.2f", a->monto);

        LVITEMA item;
        memset(&item, 0, sizeof(item));
        item.mask = LVIF_TEXT;
        item.iItem = aRow;
        item.pszText = idBuf;
        ListView_InsertItem(lvAbonos, &item);

        ListView_SetItemText(lvAbonos, aRow, 1, montoBuf);
        ListView_SetItemText(lvAbonos, aRow, 2, a->fecha ? a->fecha : "-");
        aRow++;
    }
}

static void refresh_combos_prestamos(int selectPrestamoId) {
    HWND cmbCli = GetDlgItem(g_app.hwndMain, ID_PRE_CMB_CLI);
    HWND cmbVeh = GetDlgItem(g_app.hwndMain, ID_PRE_CMB_VEH);
    HWND cmbAct = GetDlgItem(g_app.hwndMain, ID_PRE_CMB_ACTIVO);
    HWND cmbRep = GetDlgItem(g_app.hwndMain, ID_REP_CMB_PRESTAMO);

    SendMessageA(cmbCli, CB_RESETCONTENT, 0, 0);
    for (ClienteNode *c = g_app.sys->clientes; c; c = c->next) {
        char label[256];
        snprintf(label, sizeof(label), "#%d - %s (%s)", c->id, c->nombre, c->documento);
        int idx = (int)SendMessageA(cmbCli, CB_ADDSTRING, 0, (LPARAM)label);
        SendMessageA(cmbCli, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)c->id);
    }
    if (SendMessageA(cmbCli, CB_GETCOUNT, 0, 0) > 0) SendMessageA(cmbCli, CB_SETCURSEL, 0, 0);

    SendMessageA(cmbVeh, CB_RESETCONTENT, 0, 0);
    for (VehiculoNode *v = g_app.sys->vehiculos; v; v = v->next) {
        char label[256];
        snprintf(label, sizeof(label), "#%d - %s %s (%d)", v->id, v->marca, v->modelo, v->anio);
        int idx = (int)SendMessageA(cmbVeh, CB_ADDSTRING, 0, (LPARAM)label);
        SendMessageA(cmbVeh, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)v->id);
    }
    if (SendMessageA(cmbVeh, CB_GETCOUNT, 0, 0) > 0) SendMessageA(cmbVeh, CB_SETCURSEL, 0, 0);

    SendMessageA(cmbAct, CB_RESETCONTENT, 0, 0);
    SendMessageA(cmbRep, CB_RESETCONTENT, 0, 0);
    int selIdx = 0;
    int pos = 0;
    for (PrestamoNode *p = g_app.sys->prestamos; p; p = p->next) {
        ClienteNode *c = cliente_buscar_id(g_app.sys, p->clienteId);
        VehiculoNode *v = vehiculo_buscar_id(g_app.sys, p->vehiculoId);
        char label[256];
        snprintf(label, sizeof(label), "Prestamo #%d | %s | %s %s | Saldo: $%.2f",
                 p->id,
                 c ? c->nombre : "Cliente?",
                 v ? v->marca : "Veh?",
                 v ? v->modelo : "",
                 p->saldoPendiente);
        int idx1 = (int)SendMessageA(cmbAct, CB_ADDSTRING, 0, (LPARAM)label);
        SendMessageA(cmbAct, CB_SETITEMDATA, (WPARAM)idx1, (LPARAM)p->id);
        int idx2 = (int)SendMessageA(cmbRep, CB_ADDSTRING, 0, (LPARAM)label);
        SendMessageA(cmbRep, CB_SETITEMDATA, (WPARAM)idx2, (LPARAM)p->id);
        if (p->id == selectPrestamoId) selIdx = pos;
        pos++;
    }
    if (pos > 0) {
        SendMessageA(cmbAct, CB_SETCURSEL, (WPARAM)selIdx, 0);
        SendMessageA(cmbRep, CB_SETCURSEL, (WPARAM)selIdx, 0);
    }

    refresh_prestamo_detalle();
}

static void show_tab(int tabIndex) {
    g_app.currentTab = tabIndex;
    for (int i = 0; i < g_app.tab0Count; i++) ShowWindow(g_app.tab0Ctrls[i], tabIndex == 0 ? SW_SHOW : SW_HIDE);
    for (int i = 0; i < g_app.tab1Count; i++) ShowWindow(g_app.tab1Ctrls[i], tabIndex == 1 ? SW_SHOW : SW_HIDE);
    for (int i = 0; i < g_app.tab2Count; i++) ShowWindow(g_app.tab2Ctrls[i], tabIndex == 2 ? SW_SHOW : SW_HIDE);
    for (int i = 0; i < g_app.tab3Count; i++) ShowWindow(g_app.tab3Ctrls[i], tabIndex == 3 ? SW_SHOW : SW_HIDE);
}

static void build_tabs_ui(HWND hwnd) {
    g_app.hTab = CreateWindowExA(
        0, WC_TABCONTROLA, "", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        12, 10, 1040, 680, hwnd, (HMENU)(INT_PTR)ID_TAB_MAIN, GetModuleHandle(NULL), NULL
    );
    SendMessageA(g_app.hTab, WM_SETFONT, (WPARAM)g_app.hFontUI, TRUE);

    const char *tabNames[] = {"Clientes", "Vehiculos", "Prestamos y Pagos", "Reportes"};
    for (int i = 0; i < 4; i++) {
        TCITEMA tie;
        memset(&tie, 0, sizeof(tie));
        tie.mask = TCIF_TEXT;
        tie.pszText = (LPSTR)tabNames[i];
        TabCtrl_InsertItem(g_app.hTab, i, &tie);
    }

    /* ================= TAB 0: CLIENTES ================= */
    make_ctrl("STATIC", "Nombre completo:", 0, 28, 52, 120, 20, -1, g_app.tab0Ctrls, &g_app.tab0Count, 16, g_app.hFontUI);
    make_edit("", 0, 28, 74, 240, 24, ID_CLI_NOMBRE, g_app.tab0Ctrls, &g_app.tab0Count, 16);

    make_ctrl("STATIC", "Documento:", 0, 282, 52, 100, 20, -1, g_app.tab0Ctrls, &g_app.tab0Count, 16, g_app.hFontUI);
    make_edit("", 0, 282, 74, 170, 24, ID_CLI_DOC, g_app.tab0Ctrls, &g_app.tab0Count, 16);

    make_ctrl("STATIC", "Telefono:", 0, 466, 52, 100, 20, -1, g_app.tab0Ctrls, &g_app.tab0Count, 16, g_app.hFontUI);
    make_edit("", 0, 466, 74, 170, 24, ID_CLI_TEL, g_app.tab0Ctrls, &g_app.tab0Count, 16);

    make_ctrl("BUTTON", "Agregar Cliente", BS_PUSHBUTTON | WS_TABSTOP, 654, 72, 140, 28, ID_CLI_BTN_ADD, g_app.tab0Ctrls, &g_app.tab0Count, 16, g_app.hFontUI);

    make_ctrl("STATIC", "Buscar (ID, Doc o Nombre):", 0, 28, 114, 180, 20, -1, g_app.tab0Ctrls, &g_app.tab0Count, 16, g_app.hFontUI);
    make_edit("", 0, 210, 112, 260, 24, ID_CLI_SEARCH_EDIT, g_app.tab0Ctrls, &g_app.tab0Count, 16);
    make_ctrl("BUTTON", "Buscar", BS_PUSHBUTTON | WS_TABSTOP, 482, 110, 95, 28, ID_CLI_BTN_SEARCH, g_app.tab0Ctrls, &g_app.tab0Count, 16, g_app.hFontUI);
    make_ctrl("BUTTON", "Mostrar Todos", BS_PUSHBUTTON | WS_TABSTOP, 588, 110, 120, 28, ID_CLI_BTN_ALL, g_app.tab0Ctrls, &g_app.tab0Count, 16, g_app.hFontUI);

    HWND lvCli = make_listview(28, 150, 1004, 470, ID_CLI_LIST, g_app.tab0Ctrls, &g_app.tab0Count, 16);
    lv_add_col(lvCli, 0, "ID", 60);
    lv_add_col(lvCli, 1, "Nombre Completo", 380);
    lv_add_col(lvCli, 2, "Documento", 250);
    lv_add_col(lvCli, 3, "Telefono", 250);

    make_ctrl("BUTTON", "Borrar Cliente Seleccionado", BS_PUSHBUTTON | WS_TABSTOP, 28, 632, 210, 32, ID_CLI_BTN_DEL, g_app.tab0Ctrls, &g_app.tab0Count, 16, g_app.hFontUI);

    /* ================= TAB 1: VEHICULOS ================= */
    make_ctrl("STATIC", "Marca:", 0, 28, 52, 100, 20, -1, g_app.tab1Ctrls, &g_app.tab1Count, 16, g_app.hFontUI);
    make_edit("", 0, 28, 74, 210, 24, ID_VEH_MARCA, g_app.tab1Ctrls, &g_app.tab1Count, 16);

    make_ctrl("STATIC", "Modelo:", 0, 252, 52, 100, 20, -1, g_app.tab1Ctrls, &g_app.tab1Count, 16, g_app.hFontUI);
    make_edit("", 0, 252, 74, 210, 24, ID_VEH_MODELO, g_app.tab1Ctrls, &g_app.tab1Count, 16);

    make_ctrl("STATIC", "Anio:", 0, 476, 52, 80, 20, -1, g_app.tab1Ctrls, &g_app.tab1Count, 16, g_app.hFontUI);
    make_edit("", ES_NUMBER, 476, 74, 110, 24, ID_VEH_ANIO, g_app.tab1Ctrls, &g_app.tab1Count, 16);

    make_ctrl("BUTTON", "Agregar Vehiculo", BS_PUSHBUTTON | WS_TABSTOP, 604, 72, 150, 28, ID_VEH_BTN_ADD, g_app.tab1Ctrls, &g_app.tab1Count, 16, g_app.hFontUI);

    HWND lvVeh = make_listview(28, 118, 1004, 502, ID_VEH_LIST, g_app.tab1Ctrls, &g_app.tab1Count, 16);
    lv_add_col(lvVeh, 0, "ID", 70);
    lv_add_col(lvVeh, 1, "Marca", 320);
    lv_add_col(lvVeh, 2, "Modelo", 380);
    lv_add_col(lvVeh, 3, "Anio", 160);

    make_ctrl("BUTTON", "Eliminar Vehiculo Seleccionado", BS_PUSHBUTTON | WS_TABSTOP, 28, 632, 230, 32, ID_VEH_BTN_DEL, g_app.tab1Ctrls, &g_app.tab1Count, 16, g_app.hFontUI);

    /* ================= TAB 2: PRESTAMOS Y PAGOS ================= */
    make_ctrl("STATIC", "Cliente:", 0, 28, 46, 80, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_ctrl("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 28, 66, 260, 200, ID_PRE_CMB_CLI, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);

    make_ctrl("STATIC", "Vehiculo:", 0, 300, 46, 80, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_ctrl("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 300, 66, 260, 200, ID_PRE_CMB_VEH, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);

    make_ctrl("STATIC", "Monto ($):", 0, 572, 46, 90, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_edit("", 0, 572, 66, 110, 24, ID_PRE_MONTO, g_app.tab2Ctrls, &g_app.tab2Count, 32);

    make_ctrl("STATIC", "Tasa Anual (%):", 0, 692, 46, 100, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_edit("12.0", 0, 692, 66, 85, 24, ID_PRE_TASA, g_app.tab2Ctrls, &g_app.tab2Count, 32);

    make_ctrl("STATIC", "Plazo (meses):", 0, 788, 46, 100, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_edit("24", ES_NUMBER, 788, 66, 85, 24, ID_PRE_PLAZO, g_app.tab2Ctrls, &g_app.tab2Count, 32);

    make_ctrl("BUTTON", "Crear Prestamo", BS_PUSHBUTTON | WS_TABSTOP, 886, 64, 146, 28, ID_PRE_BTN_CREAR, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);

    make_ctrl("STATIC", "Prestamo Activo:", 0, 28, 104, 120, 20, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_ctrl("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 150, 100, 882, 220, ID_PRE_CMB_ACTIVO, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);

    make_ctrl("STATIC", "", SS_SUNKEN, 28, 134, 1004, 42, ID_PRE_LBL_RESUMEN, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);

    make_ctrl("STATIC", "Lista de Meses (Pagados y No Pagados):", 0, 28, 184, 300, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    HWND lvMeses = make_listview(28, 206, 630, 410, ID_PRE_LIST_MESES, g_app.tab2Ctrls, &g_app.tab2Count, 32);
    lv_add_col(lvMeses, 0, "Mes", 55);
    lv_add_col(lvMeses, 1, "Cuota", 100);
    lv_add_col(lvMeses, 2, "Capital", 100);
    lv_add_col(lvMeses, 3, "Interes", 95);
    lv_add_col(lvMeses, 4, "Estado", 110);
    lv_add_col(lvMeses, 5, "Fecha Pago", 130);

    make_ctrl("BUTTON", "Registrar Pago Mensual", BS_PUSHBUTTON | WS_TABSTOP, 28, 628, 190, 32, ID_PRE_BTN_PAGAR_MES, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_ctrl("BUTTON", "Marcar / Desmarcar Mes", BS_PUSHBUTTON | WS_TABSTOP, 232, 628, 190, 32, ID_PRE_BTN_TOGGLE_MES, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);

    /* Panel derecho de Abonos */
    make_ctrl("STATIC", "Pagos de Abono a Capital:", 0, 676, 184, 240, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_ctrl("STATIC", "Monto Abono ($):", 0, 676, 210, 120, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_edit("", 0, 676, 230, 160, 24, ID_PRE_ABONO_MONTO, g_app.tab2Ctrls, &g_app.tab2Count, 32);

    char hoy[32];
    fecha_actual_iso(hoy, sizeof(hoy));
    make_ctrl("STATIC", "Fecha (AAAA-MM-DD):", 0, 848, 210, 140, 18, -1, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);
    make_edit(hoy, 0, 848, 230, 184, 24, ID_PRE_ABONO_FECHA, g_app.tab2Ctrls, &g_app.tab2Count, 32);

    make_ctrl("BUTTON", "Registrar Abono", BS_PUSHBUTTON | WS_TABSTOP, 676, 264, 356, 30, ID_PRE_BTN_ABONO, g_app.tab2Ctrls, &g_app.tab2Count, 32, g_app.hFontUI);

    HWND lvAbonos = make_listview(676, 306, 356, 310, ID_PRE_LIST_ABONOS, g_app.tab2Ctrls, &g_app.tab2Count, 32);
    lv_add_col(lvAbonos, 0, "ID", 50);
    lv_add_col(lvAbonos, 1, "Monto Abono", 140);
    lv_add_col(lvAbonos, 2, "Fecha", 140);

    /* ================= TAB 3: REPORTES ================= */
    make_ctrl("STATIC", "Prestamo:", 0, 28, 52, 80, 20, -1, g_app.tab3Ctrls, &g_app.tab3Count, 16, g_app.hFontUI);
    make_ctrl("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 108, 48, 580, 220, ID_REP_CMB_PRESTAMO, g_app.tab3Ctrls, &g_app.tab3Count, 16, g_app.hFontUI);
    make_ctrl("BUTTON", "Generar Reporte", BS_PUSHBUTTON | WS_TABSTOP, 702, 46, 150, 28, ID_REP_BTN_GENERAR, g_app.tab3Ctrls, &g_app.tab3Count, 16, g_app.hFontUI);
    make_ctrl("BUTTON", "Exportar a TXT", BS_PUSHBUTTON | WS_TABSTOP, 864, 46, 168, 28, ID_REP_BTN_EXPORTAR, g_app.tab3Ctrls, &g_app.tab3Count, 16, g_app.hFontUI);

    HWND editRep = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
        28, 90, 1004, 575, hwnd, (HMENU)(INT_PTR)ID_REP_EDIT_TEXTO, GetModuleHandle(NULL), NULL
    );
    SendMessageA(editRep, WM_SETFONT, (WPARAM)g_app.hFontMono, TRUE);
    add_tab_ctrl(g_app.tab3Ctrls, &g_app.tab3Count, 16, editRep);

    show_tab(0);
    refresh_clientes_list("");
    refresh_vehiculos_list();
    refresh_combos_prestamos(-1);
}

static void handle_command(WPARAM wParam) {
    int id = LOWORD(wParam);
    int code = HIWORD(wParam);
    char err[256] = {0};

    switch (id) {
        case ID_CLI_BTN_ADD: {
            char nombre[128], doc[64], tel[64];
            GetDlgItemTextA(g_app.hwndMain, ID_CLI_NOMBRE, nombre, sizeof(nombre));
            GetDlgItemTextA(g_app.hwndMain, ID_CLI_DOC, doc, sizeof(doc));
            GetDlgItemTextA(g_app.hwndMain, ID_CLI_TEL, tel, sizeof(tel));

            if (!cliente_agregar(g_app.sys, nombre, doc, tel, err, sizeof(err))) {
                MessageBoxA(g_app.hwndMain, err, "Validacion de Cliente", MB_OK | MB_ICONWARNING);
                break;
            }
            SetDlgItemTextA(g_app.hwndMain, ID_CLI_NOMBRE, "");
            SetDlgItemTextA(g_app.hwndMain, ID_CLI_DOC, "");
            SetDlgItemTextA(g_app.hwndMain, ID_CLI_TEL, "");
            refresh_clientes_list("");
            refresh_combos_prestamos(-1);
            guardar_datos();
            break;
        }
        case ID_CLI_BTN_SEARCH: {
            char criterio[128];
            GetDlgItemTextA(g_app.hwndMain, ID_CLI_SEARCH_EDIT, criterio, sizeof(criterio));
            refresh_clientes_list(criterio);
            break;
        }
        case ID_CLI_BTN_ALL: {
            SetDlgItemTextA(g_app.hwndMain, ID_CLI_SEARCH_EDIT, "");
            refresh_clientes_list("");
            break;
        }
        case ID_CLI_BTN_DEL: {
            HWND lv = GetDlgItem(g_app.hwndMain, ID_CLI_LIST);
            int cliId = get_selected_lv_id(lv);
            if (cliId < 0) {
                MessageBoxA(g_app.hwndMain, "Seleccione un cliente de la lista.", "Aviso", MB_OK | MB_ICONINFORMATION);
                break;
            }
            if (!cliente_eliminar(g_app.sys, cliId, err, sizeof(err))) {
                MessageBoxA(g_app.hwndMain, err, "No se pudo borrar", MB_OK | MB_ICONWARNING);
                break;
            }
            refresh_clientes_list("");
            refresh_combos_prestamos(-1);
            guardar_datos();
            break;
        }
        case ID_VEH_BTN_ADD: {
            char marca[128], modelo[128], anioStr[32];
            GetDlgItemTextA(g_app.hwndMain, ID_VEH_MARCA, marca, sizeof(marca));
            GetDlgItemTextA(g_app.hwndMain, ID_VEH_MODELO, modelo, sizeof(modelo));
            GetDlgItemTextA(g_app.hwndMain, ID_VEH_ANIO, anioStr, sizeof(anioStr));
            int anio = atoi(anioStr);

            if (!vehiculo_agregar(g_app.sys, marca, modelo, anio, err, sizeof(err))) {
                MessageBoxA(g_app.hwndMain, err, "Validacion de Vehiculo", MB_OK | MB_ICONWARNING);
                break;
            }
            SetDlgItemTextA(g_app.hwndMain, ID_VEH_MARCA, "");
            SetDlgItemTextA(g_app.hwndMain, ID_VEH_MODELO, "");
            SetDlgItemTextA(g_app.hwndMain, ID_VEH_ANIO, "");
            refresh_vehiculos_list();
            refresh_combos_prestamos(-1);
            guardar_datos();
            break;
        }
        case ID_VEH_BTN_DEL: {
            HWND lv = GetDlgItem(g_app.hwndMain, ID_VEH_LIST);
            int vehId = get_selected_lv_id(lv);
            if (vehId < 0) {
                MessageBoxA(g_app.hwndMain, "Seleccione un vehiculo de la lista.", "Aviso", MB_OK | MB_ICONINFORMATION);
                break;
            }
            if (!vehiculo_eliminar(g_app.sys, vehId, err, sizeof(err))) {
                MessageBoxA(g_app.hwndMain, err, "No se pudo eliminar", MB_OK | MB_ICONWARNING);
                break;
            }
            refresh_vehiculos_list();
            refresh_combos_prestamos(-1);
            guardar_datos();
            break;
        }
        case ID_PRE_BTN_CREAR: {
            int cliId = get_combo_selected_item_data(GetDlgItem(g_app.hwndMain, ID_PRE_CMB_CLI));
            int vehId = get_combo_selected_item_data(GetDlgItem(g_app.hwndMain, ID_PRE_CMB_VEH));
            char montoStr[64], tasaStr[64], plazoStr[32];
            GetDlgItemTextA(g_app.hwndMain, ID_PRE_MONTO, montoStr, sizeof(montoStr));
            GetDlgItemTextA(g_app.hwndMain, ID_PRE_TASA, tasaStr, sizeof(tasaStr));
            GetDlgItemTextA(g_app.hwndMain, ID_PRE_PLAZO, plazoStr, sizeof(plazoStr));

            double monto = atof(montoStr);
            double tasa = atof(tasaStr);
            int plazo = atoi(plazoStr);

            PrestamoNode *nuevo = prestamo_crear(g_app.sys, cliId, vehId, monto, tasa, plazo, err, sizeof(err));
            if (!nuevo) {
                MessageBoxA(g_app.hwndMain, err, "Validacion de Prestamo", MB_OK | MB_ICONWARNING);
                break;
            }
            SetDlgItemTextA(g_app.hwndMain, ID_PRE_MONTO, "");
            refresh_combos_prestamos(nuevo->id);
            guardar_datos();
            MessageBoxA(g_app.hwndMain, "Prestamo creado exitosamente.", "Confirmacion", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case ID_PRE_CMB_ACTIVO: {
            if (code == CBN_SELCHANGE) {
                refresh_prestamo_detalle();
            }
            break;
        }
        case ID_PRE_BTN_PAGAR_MES:
        case ID_PRE_BTN_TOGGLE_MES: {
            int prestamoId = get_combo_selected_item_data(GetDlgItem(g_app.hwndMain, ID_PRE_CMB_ACTIVO));
            PrestamoNode *p = prestamo_buscar_id(g_app.sys, prestamoId);
            if (!p) {
                MessageBoxA(g_app.hwndMain, "No hay un prestamo seleccionado.", "Aviso", MB_OK | MB_ICONINFORMATION);
                break;
            }
            int numMes = get_selected_lv_id(GetDlgItem(g_app.hwndMain, ID_PRE_LIST_MESES));
            if (numMes <= 0) {
                MessageBoxA(g_app.hwndMain, "Seleccione un mes de la lista.", "Aviso", MB_OK | MB_ICONINFORMATION);
                break;
            }

            int ok = 0;
            if (id == ID_PRE_BTN_PAGAR_MES) {
                ok = prestamo_pagar_mes(p, numMes, NULL, err, sizeof(err));
            } else {
                MesPagoNode *m = prestamo_buscar_mes(p, numMes);
                int nuevoEstado = (m && m->pagado) ? 0 : 1;
                ok = prestamo_marcar_mes(p, numMes, nuevoEstado, NULL, err, sizeof(err));
            }

            if (!ok) {
                MessageBoxA(g_app.hwndMain, err, "Pago Mensual", MB_OK | MB_ICONWARNING);
                break;
            }
            refresh_combos_prestamos(p->id);
            guardar_datos();
            break;
        }
        case ID_PRE_BTN_ABONO: {
            int prestamoId = get_combo_selected_item_data(GetDlgItem(g_app.hwndMain, ID_PRE_CMB_ACTIVO));
            PrestamoNode *p = prestamo_buscar_id(g_app.sys, prestamoId);
            if (!p) {
                MessageBoxA(g_app.hwndMain, "No hay un prestamo seleccionado.", "Aviso", MB_OK | MB_ICONINFORMATION);
                break;
            }
            char montoStr[64], fechaStr[32];
            GetDlgItemTextA(g_app.hwndMain, ID_PRE_ABONO_MONTO, montoStr, sizeof(montoStr));
            GetDlgItemTextA(g_app.hwndMain, ID_PRE_ABONO_FECHA, fechaStr, sizeof(fechaStr));
            double monto = atof(montoStr);

            if (!prestamo_agregar_abono(p, monto, fechaStr, err, sizeof(err))) {
                MessageBoxA(g_app.hwndMain, err, "Validacion de Abono", MB_OK | MB_ICONWARNING);
                break;
            }
            SetDlgItemTextA(g_app.hwndMain, ID_PRE_ABONO_MONTO, "");
            refresh_combos_prestamos(p->id);
            guardar_datos();
            break;
        }
        case ID_REP_BTN_GENERAR: {
            int prestamoId = get_combo_selected_item_data(GetDlgItem(g_app.hwndMain, ID_REP_CMB_PRESTAMO));
            char *texto = reporte_generar_texto(g_app.sys, prestamoId);
            if (!texto) {
                MessageBoxA(g_app.hwndMain, "Seleccione un prestamo valido para generar el reporte.", "Aviso", MB_OK | MB_ICONINFORMATION);
                break;
            }
            SetDlgItemTextA(g_app.hwndMain, ID_REP_EDIT_TEXTO, texto);
            free(texto);
            break;
        }
        case ID_REP_BTN_EXPORTAR: {
            int prestamoId = get_combo_selected_item_data(GetDlgItem(g_app.hwndMain, ID_REP_CMB_PRESTAMO));
            if (prestamoId < 0) {
                MessageBoxA(g_app.hwndMain, "Seleccione un prestamo valido.", "Aviso", MB_OK | MB_ICONINFORMATION);
                break;
            }
            char ruta[128];
            snprintf(ruta, sizeof(ruta), "data/reporte_prestamo_%d.txt", prestamoId);
            if (!reporte_exportar_txt(g_app.sys, prestamoId, ruta, err, sizeof(err))) {
                MessageBoxA(g_app.hwndMain, err, "Error al exportar", MB_OK | MB_ICONERROR);
                break;
            }
            char msg[256];
            snprintf(msg, sizeof(msg), "Reporte exportado correctamente en:\n%s", ruta);
            MessageBoxA(g_app.hwndMain, msg, "Reporte Exportado", MB_OK | MB_ICONINFORMATION);
            break;
        }
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_app.hwndMain = hwnd;
            g_app.hFontUI = CreateFontA(
                17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI"
            );
            g_app.hFontMono = CreateFontA(
                15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas"
            );
            build_tabs_ui(hwnd);
            return 0;
        }
        case WM_NOTIFY: {
            LPNMHDR nm = (LPNMHDR)lParam;
            if (nm && nm->idFrom == ID_TAB_MAIN && nm->code == TCN_SELCHANGE) {
                int sel = TabCtrl_GetCurSel(g_app.hTab);
                show_tab(sel);
            }
            return 0;
        }
        case WM_COMMAND:
            handle_command(wParam);
            return 0;
        case WM_CLOSE:
            guardar_datos();
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            if (g_app.hFontUI) DeleteObject(g_app.hFontUI);
            if (g_app.hFontMono) DeleteObject(g_app.hFontMono);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int gui_run(HINSTANCE hInstance, int nCmdShow, SistemaPrestamos *sys) {
    memset(&g_app, 0, sizeof(g_app));
    g_app.sys = sys;

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    const char *CLASS_NAME = "PrestamosVehiculosWnd";
    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExA(&wc)) {
        return 1;
    }

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winW = 1080;
    int winH = 740;
    int posX = (screenW - winW) / 2;
    int posY = (screenH - winH) / 2;

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "Sistema de Prestamos Vehiculares",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, winW, winH,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    return (int)msg.wParam;
}
