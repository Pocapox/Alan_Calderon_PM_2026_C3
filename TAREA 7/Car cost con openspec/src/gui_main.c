#define WIN32_LEAN_AND_MEAN
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calculator.h"
#include "parse.h"
#include "storage.h"
#include "utf8.h"
#include "vehicle.h"

#define ID_TAB_CONTROL          1000

/* Tab 0 - Vehiculos IDs */
#define ID_LV_VEHICLES          1001
#define ID_BTN_NEW              1002
#define ID_BTN_SAVE             1003
#define ID_BTN_DELETE           1004

#define ID_EDIT_FORM_BASE       1100
/* 13 campos: 1100 a 1112 */

/* Tab 1 - Calcular viaje IDs */
#define ID_COMBO_CALC_VEHICLE   1200
#define ID_EDIT_CALC_DIST       1201
#define ID_EDIT_CALC_CITY_PCT   1202
#define ID_EDIT_CALC_HWY_PCT    1203
#define ID_EDIT_CALC_PRICE      1204
#define ID_BTN_CALCULATE        1205
#define ID_EDIT_CALC_RESULT     1206

#define NUM_FORM_FIELDS 13

static const wchar_t *g_field_labels[NUM_FORM_FIELDS] = {
    L"Nombre del vehiculo:",
    L"Consumo ciudad (km/L):",
    L"Consumo carretera (km/L):",
    L"Seguro anual (DOP):",
    L"Neumaticos por km (DOP):",
    L"Mantenimiento por km (DOP):",
    L"Valor inicial (DOP):",
    L"Anios de vida util:",
    L"Valor final / residual (DOP):",
    L"Impuestos anuales (DOP):",
    L"Costo por lavado (DOP):",
    L"Lavados por anio:",
    L"Kilometraje anual promedio (km):"
};

typedef struct {
    HWND hwnd_main;
    HWND hwnd_tab;
    HFONT font_ui;
    HFONT font_mono;
    VehicleStore store;
    int selected_index; /* -1: nuevo / ninguno */
    int active_tab;     /* 0: Vehiculos, 1: Calcular */
    int updating_selection;

    /* Tab 0 Controls */
    HWND hwnd_lbl_list;
    HWND hwnd_list;
    HWND hwnd_btn_new;
    HWND hwnd_btn_save;
    HWND hwnd_btn_delete;

    HWND hwnd_form_group;
    HWND hwnd_lbl[NUM_FORM_FIELDS];
    HWND hwnd_edit[NUM_FORM_FIELDS];

    /* Tab 1 Controls */
    HWND hwnd_calc_group;
    HWND hwnd_calc_lbl_veh;
    HWND hwnd_calc_combo_veh;
    HWND hwnd_calc_lbl_dist;
    HWND hwnd_calc_edit_dist;
    HWND hwnd_calc_lbl_city;
    HWND hwnd_calc_edit_city;
    HWND hwnd_calc_lbl_hwy;
    HWND hwnd_calc_edit_hwy;
    HWND hwnd_calc_lbl_price;
    HWND hwnd_calc_edit_price;
    HWND hwnd_calc_btn_compute;
    HWND hwnd_calc_lbl_result;
    HWND hwnd_calc_edit_result;
} AppState;

static void show_error(HWND hwnd, const wchar_t *message, const wchar_t *title)
{
    MessageBoxW(hwnd, message, title != NULL ? title : L"Error", MB_ICONERROR | MB_OK);
}

static void show_info(HWND hwnd, const wchar_t *message, const wchar_t *title)
{
    MessageBoxW(hwnd, message, title != NULL ? title : L"Informacion", MB_ICONINFORMATION | MB_OK);
}

static void set_ctrl_font(HWND hwnd, HFONT font)
{
    if (hwnd != NULL && font != NULL) {
        SendMessageW(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
    }
}

static void update_tab_visibility(AppState *app)
{
    int show_tab0 = (app->active_tab == 0) ? SW_SHOW : SW_HIDE;
    int show_tab1 = (app->active_tab == 1) ? SW_SHOW : SW_HIDE;
    int i;

    /* Tab 0 controls */
    ShowWindow(app->hwnd_lbl_list, show_tab0);
    ShowWindow(app->hwnd_list, show_tab0);
    ShowWindow(app->hwnd_btn_new, show_tab0);
    ShowWindow(app->hwnd_btn_save, show_tab0);
    ShowWindow(app->hwnd_btn_delete, show_tab0);
    ShowWindow(app->hwnd_form_group, show_tab0);
    for (i = 0; i < NUM_FORM_FIELDS; i++) {
        ShowWindow(app->hwnd_lbl[i], show_tab0);
        ShowWindow(app->hwnd_edit[i], show_tab0);
    }

    /* Tab 1 controls */
    ShowWindow(app->hwnd_calc_group, show_tab1);
    ShowWindow(app->hwnd_calc_lbl_veh, show_tab1);
    ShowWindow(app->hwnd_calc_combo_veh, show_tab1);
    ShowWindow(app->hwnd_calc_lbl_dist, show_tab1);
    ShowWindow(app->hwnd_calc_edit_dist, show_tab1);
    ShowWindow(app->hwnd_calc_lbl_city, show_tab1);
    ShowWindow(app->hwnd_calc_edit_city, show_tab1);
    ShowWindow(app->hwnd_calc_lbl_hwy, show_tab1);
    ShowWindow(app->hwnd_calc_edit_hwy, show_tab1);
    ShowWindow(app->hwnd_calc_lbl_price, show_tab1);
    ShowWindow(app->hwnd_calc_edit_price, show_tab1);
    ShowWindow(app->hwnd_calc_btn_compute, show_tab1);
    ShowWindow(app->hwnd_calc_lbl_result, show_tab1);
    ShowWindow(app->hwnd_calc_edit_result, show_tab1);
}

static void clear_form(AppState *app)
{
    int i;
    for (i = 0; i < NUM_FORM_FIELDS; i++) {
        SetWindowTextW(app->hwnd_edit[i], L"");
    }
}

static void load_vehicle_to_form(AppState *app, int index)
{
    wchar_t wbuf[256];
    const Vehicle *v;

    if (index < 0 || index >= app->store.count) {
        clear_form(app);
        EnableWindow(app->hwnd_btn_delete, FALSE);
        return;
    }
    v = &app->store.items[index];

    utf8_to_wide(v->name, wbuf, sizeof(wbuf) / sizeof(wbuf[0]));
    SetWindowTextW(app->hwnd_edit[0], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->city_km_per_l);
    SetWindowTextW(app->hwnd_edit[1], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->highway_km_per_l);
    SetWindowTextW(app->hwnd_edit[2], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->insurance_per_year);
    SetWindowTextW(app->hwnd_edit[3], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->tires_per_km);
    SetWindowTextW(app->hwnd_edit[4], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->maintenance_per_km);
    SetWindowTextW(app->hwnd_edit[5], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->initial_value);
    SetWindowTextW(app->hwnd_edit[6], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->years_of_use);
    SetWindowTextW(app->hwnd_edit[7], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->final_value);
    SetWindowTextW(app->hwnd_edit[8], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->taxes_per_year);
    SetWindowTextW(app->hwnd_edit[9], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->wash_cost);
    SetWindowTextW(app->hwnd_edit[10], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->washes_per_year);
    SetWindowTextW(app->hwnd_edit[11], wbuf);

    swprintf(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%.2f", v->km_per_year);
    SetWindowTextW(app->hwnd_edit[12], wbuf);

    EnableWindow(app->hwnd_btn_delete, TRUE);
}

static void refresh_list_view(AppState *app)
{
    LVITEMW lvi;
    wchar_t wname[256];
    wchar_t wcity[64];
    wchar_t whwy[64];
    int i;

    app->updating_selection = 1;
    SendMessageW(app->hwnd_list, LVM_DELETEALLITEMS, 0, 0);

    for (i = 0; i < app->store.count; i++) {
        const Vehicle *v = &app->store.items[i];
        utf8_to_wide(v->name, wname, sizeof(wname) / sizeof(wname[0]));
        swprintf(wcity, sizeof(wcity) / sizeof(wcity[0]), L"%.2f", v->city_km_per_l);
        swprintf(whwy, sizeof(whwy) / sizeof(whwy[0]), L"%.2f", v->highway_km_per_l);

        memset(&lvi, 0, sizeof(lvi));
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.pszText = wname;
        SendMessageW(app->hwnd_list, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

        lvi.iSubItem = 1;
        lvi.pszText = wcity;
        SendMessageW(app->hwnd_list, LVM_SETITEMTEXTW, i, (LPARAM)&lvi);

        lvi.iSubItem = 2;
        lvi.pszText = whwy;
        SendMessageW(app->hwnd_list, LVM_SETITEMTEXTW, i, (LPARAM)&lvi);
    }

    if (app->selected_index >= 0 && app->selected_index < app->store.count) {
        memset(&lvi, 0, sizeof(lvi));
        lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
        SendMessageW(app->hwnd_list, LVM_SETITEMSTATE, app->selected_index, (LPARAM)&lvi);
        SendMessageW(app->hwnd_list, LVM_ENSUREVISIBLE, app->selected_index, FALSE);
        load_vehicle_to_form(app, app->selected_index);
    } else {
        clear_form(app);
        EnableWindow(app->hwnd_btn_delete, FALSE);
    }
    app->updating_selection = 0;
}

static void populate_calc_combo(AppState *app)
{
    int current_sel = (int)SendMessageW(app->hwnd_calc_combo_veh, CB_GETCURSEL, 0, 0);
    wchar_t wname[256];
    int i;

    SendMessageW(app->hwnd_calc_combo_veh, CB_RESETCONTENT, 0, 0);
    for (i = 0; i < app->store.count; i++) {
        utf8_to_wide(app->store.items[i].name, wname, sizeof(wname) / sizeof(wname[0]));
        SendMessageW(app->hwnd_calc_combo_veh, CB_ADDSTRING, 0, (LPARAM)wname);
    }

    if (app->store.count > 0) {
        if (current_sel >= 0 && current_sel < app->store.count) {
            SendMessageW(app->hwnd_calc_combo_veh, CB_SETCURSEL, current_sel, 0);
        } else if (app->selected_index >= 0 && app->selected_index < app->store.count) {
            SendMessageW(app->hwnd_calc_combo_veh, CB_SETCURSEL, app->selected_index, 0);
        } else {
            SendMessageW(app->hwnd_calc_combo_veh, CB_SETCURSEL, 0, 0);
        }
        EnableWindow(app->hwnd_calc_btn_compute, TRUE);
    } else {
        EnableWindow(app->hwnd_calc_btn_compute, FALSE);
    }
}

static void on_btn_new(AppState *app)
{
    app->updating_selection = 1;
    app->selected_index = -1;
    SendMessageW(app->hwnd_list, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)0);
    app->updating_selection = 0;

    clear_form(app);
    EnableWindow(app->hwnd_btn_delete, FALSE);
    SetFocus(app->hwnd_edit[0]);
}

static void on_btn_save(AppState *app)
{
    wchar_t wtext[256];
    char text[256];
    char err_buf[256];
    Vehicle v;
    double *targets[12];
    int i;

    memset(&v, 0, sizeof(v));

    /* 1. Nombre */
    GetWindowTextW(app->hwnd_edit[0], wtext, sizeof(wtext) / sizeof(wtext[0]));
    if (!wide_to_utf8(wtext, v.name, sizeof(v.name))) {
        show_error(app->hwnd_main, L"El nombre ingresado es invalido o excede el limite permitido.", L"Error de validacion");
        return;
    }
    parse_trim(v.name);

    if (!vehicle_name_is_valid(v.name, err_buf, sizeof(err_buf))) {
        utf8_to_wide(err_buf, wtext, sizeof(wtext) / sizeof(wtext[0]));
        show_error(app->hwnd_main, wtext, L"Error de validacion");
        SetFocus(app->hwnd_edit[0]);
        return;
    }

    if (vehicle_name_exists(&app->store, v.name, app->selected_index)) {
        show_error(app->hwnd_main, L"Ya existe un vehiculo con ese nombre.", L"Error de validacion");
        SetFocus(app->hwnd_edit[0]);
        return;
    }

    /* 2. Parametros numericos */
    targets[0] = &v.city_km_per_l;
    targets[1] = &v.highway_km_per_l;
    targets[2] = &v.insurance_per_year;
    targets[3] = &v.tires_per_km;
    targets[4] = &v.maintenance_per_km;
    targets[5] = &v.initial_value;
    targets[6] = &v.years_of_use;
    targets[7] = &v.final_value;
    targets[8] = &v.taxes_per_year;
    targets[9] = &v.wash_cost;
    targets[10] = &v.washes_per_year;
    targets[11] = &v.km_per_year;

    for (i = 0; i < 12; i++) {
        GetWindowTextW(app->hwnd_edit[i + 1], wtext, sizeof(wtext) / sizeof(wtext[0]));
        if (!wide_to_utf8(wtext, text, sizeof(text)) || !parse_double(text, targets[i])) {
            wchar_t msg[300];
            swprintf(msg, sizeof(msg) / sizeof(msg[0]),
                     L"Valor invalido en el campo \"%ls\". Ingrese un numero valido.",
                     g_field_labels[i + 1]);
            show_error(app->hwnd_main, msg, L"Error de validacion");
            SetFocus(app->hwnd_edit[i + 1]);
            return;
        }
    }

    if (!vehicle_validate(&v, err_buf, sizeof(err_buf))) {
        utf8_to_wide(err_buf, wtext, sizeof(wtext) / sizeof(wtext[0]));
        show_error(app->hwnd_main, wtext, L"Error de validacion");
        return;
    }

    if (app->selected_index < 0) {
        /* Crear nuevo */
        if (!vehicle_store_add(&app->store, &v, err_buf, sizeof(err_buf))) {
            utf8_to_wide(err_buf, wtext, sizeof(wtext) / sizeof(wtext[0]));
            show_error(app->hwnd_main, wtext, L"Error al crear vehiculo");
            return;
        }
        if (storage_save(STORAGE_DEFAULT_PATH, &app->store, err_buf, sizeof(err_buf)) != 0) {
            vehicle_store_remove(&app->store, app->store.count - 1);
            utf8_to_wide(err_buf, wtext, sizeof(wtext) / sizeof(wtext[0]));
            show_error(app->hwnd_main, wtext, L"Error al guardar archivo");
            return;
        }
        app->selected_index = app->store.count - 1;
        refresh_list_view(app);
        show_info(app->hwnd_main, L"Vehiculo guardado correctamente.", L"Exito");
    } else {
        /* Editar existente */
        Vehicle original = app->store.items[app->selected_index];
        if (!vehicle_store_update(&app->store, app->selected_index, &v, err_buf, sizeof(err_buf))) {
            utf8_to_wide(err_buf, wtext, sizeof(wtext) / sizeof(wtext[0]));
            show_error(app->hwnd_main, wtext, L"Error al actualizar vehiculo");
            return;
        }
        if (storage_save(STORAGE_DEFAULT_PATH, &app->store, err_buf, sizeof(err_buf)) != 0) {
            app->store.items[app->selected_index] = original;
            utf8_to_wide(err_buf, wtext, sizeof(wtext) / sizeof(wtext[0]));
            show_error(app->hwnd_main, wtext, L"Error al guardar archivo");
            return;
        }
        refresh_list_view(app);
        show_info(app->hwnd_main, L"Vehiculo actualizado correctamente.", L"Exito");
    }
}

static void on_btn_delete(AppState *app)
{
    wchar_t prompt[512];
    wchar_t wname[256];
    char err_buf[256];
    Vehicle original;
    int index = app->selected_index;
    int i;

    if (index < 0 || index >= app->store.count) {
        return;
    }

    utf8_to_wide(app->store.items[index].name, wname, sizeof(wname) / sizeof(wname[0]));
    swprintf(prompt, sizeof(prompt) / sizeof(prompt[0]),
             L"¿Esta seguro de que desea eliminar el vehiculo \"%ls\"?", wname);

    if (MessageBoxW(app->hwnd_main, prompt, L"Confirmar eliminacion",
                    MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
        return;
    }

    original = app->store.items[index];
    if (!vehicle_store_remove(&app->store, index)) {
        show_error(app->hwnd_main, L"No se pudo eliminar el vehiculo del catalogo.", L"Error");
        return;
    }

    if (storage_save(STORAGE_DEFAULT_PATH, &app->store, err_buf, sizeof(err_buf)) != 0) {
        /* Restaurar */
        for (i = app->store.count; i > index; i--) {
            app->store.items[i] = app->store.items[i - 1];
        }
        app->store.items[index] = original;
        app->store.count++;
        utf8_to_wide(err_buf, prompt, sizeof(prompt) / sizeof(prompt[0]));
        show_error(app->hwnd_main, prompt, L"Error al guardar archivo");
        return;
    }

    if (app->store.count == 0) {
        app->selected_index = -1;
    } else if (index >= app->store.count) {
        app->selected_index = app->store.count - 1;
    } else {
        app->selected_index = index;
    }

    refresh_list_view(app);
    show_info(app->hwnd_main, L"Vehiculo eliminado correctamente.", L"Exito");
}

static void on_btn_calculate(AppState *app)
{
    wchar_t wbuf[256];
    char cbuf[256];
    char err_buf[256];
    int sel_index;
    double distance;
    double city_pct;
    double hwy_pct;
    double fuel_price;
    TripCost result;

    /* Limpiar resultado anterior siempre */
    SetWindowTextW(app->hwnd_calc_edit_result, L"");

    sel_index = (int)SendMessageW(app->hwnd_calc_combo_veh, CB_GETCURSEL, 0, 0);
    if (sel_index < 0 || sel_index >= app->store.count) {
        show_error(app->hwnd_main, L"Por favor seleccione un vehiculo de la lista.", L"Error de calculo");
        return;
    }

    /* Distancia */
    GetWindowTextW(app->hwnd_calc_edit_dist, wbuf, sizeof(wbuf) / sizeof(wbuf[0]));
    if (!wide_to_utf8(wbuf, cbuf, sizeof(cbuf)) || !parse_double(cbuf, &distance)) {
        show_error(app->hwnd_main, L"Distancia de viaje invalida. Ingrese un valor numerico.", L"Error de calculo");
        SetFocus(app->hwnd_calc_edit_dist);
        return;
    }

    /* Porcentaje urbano */
    GetWindowTextW(app->hwnd_calc_edit_city, wbuf, sizeof(wbuf) / sizeof(wbuf[0]));
    if (!wide_to_utf8(wbuf, cbuf, sizeof(cbuf)) || !parse_double(cbuf, &city_pct)) {
        show_error(app->hwnd_main, L"Porcentaje urbano invalido. Ingrese un numero entre 0 y 100.", L"Error de calculo");
        SetFocus(app->hwnd_calc_edit_city);
        return;
    }

    /* Porcentaje carretera */
    GetWindowTextW(app->hwnd_calc_edit_hwy, wbuf, sizeof(wbuf) / sizeof(wbuf[0]));
    if (!wide_to_utf8(wbuf, cbuf, sizeof(cbuf)) || !parse_double(cbuf, &hwy_pct)) {
        show_error(app->hwnd_main, L"Porcentaje de carretera invalido. Ingrese un numero entre 0 y 100.", L"Error de calculo");
        SetFocus(app->hwnd_calc_edit_hwy);
        return;
    }

    /* Precio combustible */
    GetWindowTextW(app->hwnd_calc_edit_price, wbuf, sizeof(wbuf) / sizeof(wbuf[0]));
    if (!wide_to_utf8(wbuf, cbuf, sizeof(cbuf)) || !parse_double(cbuf, &fuel_price)) {
        show_error(app->hwnd_main, L"Precio del combustible invalido. Ingrese un valor numerico.", L"Error de calculo");
        SetFocus(app->hwnd_calc_edit_price);
        return;
    }

    if (calculator_compute(&app->store.items[sel_index], distance, city_pct, hwy_pct,
                           fuel_price, &result, err_buf, sizeof(err_buf)) != CALC_OK) {
        utf8_to_wide(err_buf, wbuf, sizeof(wbuf) / sizeof(wbuf[0]));
        show_error(app->hwnd_main, wbuf, L"Error de calculo");
        return;
    }

    {
        wchar_t wveh_name[256];
        wchar_t report[4096];
        utf8_to_wide(app->store.items[sel_index].name, wveh_name, sizeof(wveh_name) / sizeof(wveh_name[0]));

        swprintf(report, sizeof(report) / sizeof(report[0]),
                 L"===========================================================\r\n"
                 L"                  DESGLOSE DEL VIAJE\r\n"
                 L"===========================================================\r\n\r\n"
                 L"  Vehiculo:               %ls\r\n"
                 L"  Distancia total:        %.2f km\r\n"
                 L"  Distribucion:           %.1f%% urbano / %.1f%% carretera\r\n"
                 L"  Precio combustible:     %.2f DOP / Litro\r\n\r\n"
                 L"-----------------------------------------------------------\r\n"
                 L"  COSTO TOTAL DEL VIAJE:  %.2f DOP\r\n"
                 L"  COSTO POR KILOMETRO:    %.2f DOP / km\r\n"
                 L"-----------------------------------------------------------\r\n\r\n"
                 L"  Desglose por rubros:\r\n"
                 L"    - Combustible:                   %12.2f DOP\r\n"
                 L"    - Neumaticos:                    %12.2f DOP\r\n"
                 L"    - Mantenimiento preventivo:      %12.2f DOP\r\n"
                 L"    - Seguro (prorrateado):          %12.2f DOP\r\n"
                 L"    - Impuestos (prorrateados):      %12.2f DOP\r\n"
                 L"    - Depreciacion (prorrateada):    %12.2f DOP\r\n"
                 L"    - Limpieza (prorrateada):        %12.2f DOP\r\n\r\n"
                 L"===========================================================\r\n",
                 wveh_name, distance, city_pct, hwy_pct, fuel_price,
                 result.total_cost, result.cost_per_km,
                 result.fuel_cost, result.tires_cost, result.maintenance_cost,
                 result.insurance_cost, result.taxes_cost,
                 result.depreciation_cost, result.cleaning_cost);

        SetWindowTextW(app->hwnd_calc_edit_result, report);
    }
}

static void layout_controls(AppState *app, int width, int height)
{
    RECT tab_rect;
    int margin = 10;
    int tab_x = margin;
    int tab_y = margin;
    int tab_w = width - (2 * margin);
    int tab_h = height - (2 * margin);
    int client_x, client_y, client_w, client_h;

    if (tab_w < 100 || tab_h < 100) return;

    MoveWindow(app->hwnd_tab, tab_x, tab_y, tab_w, tab_h, TRUE);

    tab_rect.left = tab_x;
    tab_rect.top = tab_y;
    tab_rect.right = tab_x + tab_w;
    tab_rect.bottom = tab_y + tab_h;
    TabCtrl_AdjustRect(app->hwnd_tab, FALSE, &tab_rect);

    client_x = tab_rect.left + 5;
    client_y = tab_rect.top + 5;
    client_w = tab_rect.right - tab_rect.left - 10;
    client_h = tab_rect.bottom - tab_rect.top - 10;

    /* --- LAYOUT TAB 0 (Vehículos) --- */
    {
        int list_w = (client_w * 42) / 100;
        int form_x = client_x + list_w + 12;
        int form_w = client_w - list_w - 12;
        int list_h = client_h - 22;
        int cur_y;
        int i;
        int lbl_w = 200;
        int edit_w = form_w - lbl_w - 30;
        int row_h = 22;
        int row_gap = 5;

        if (edit_w < 120) {
            edit_w = 120;
            lbl_w = form_w - edit_w - 30;
        }

        /* Lista a la izquierda */
        MoveWindow(app->hwnd_lbl_list, client_x, client_y, list_w, 18, TRUE);
        MoveWindow(app->hwnd_list, client_x, client_y + 20, list_w, list_h, TRUE);

        /* Formulario a la derecha */
        MoveWindow(app->hwnd_form_group, form_x, client_y, form_w, client_h, TRUE);

        cur_y = client_y + 22;
        for (i = 0; i < NUM_FORM_FIELDS; i++) {
            MoveWindow(app->hwnd_lbl[i], form_x + 12, cur_y + 2, lbl_w, row_h, TRUE);
            MoveWindow(app->hwnd_edit[i], form_x + 12 + lbl_w, cur_y, edit_w, row_h, TRUE);
            cur_y += row_h + row_gap;
        }

        /* Botones Nuevo, Guardar, Eliminar */
        cur_y += 10;
        MoveWindow(app->hwnd_btn_new, form_x + 12, cur_y, 90, 28, TRUE);
        MoveWindow(app->hwnd_btn_save, form_x + 12 + 100, cur_y, 100, 28, TRUE);
        MoveWindow(app->hwnd_btn_delete, form_x + 12 + 210, cur_y, 90, 28, TRUE);
    }

    /* --- LAYOUT TAB 1 (Calcular viaje) --- */
    {
        int input_w = 380;
        int res_x = client_x + input_w + 14;
        int res_w = client_w - input_w - 14;
        int cur_y = client_y + 24;
        int lbl_w = 160;
        int edit_w = 190;
        int row_h = 23;
        int row_gap = 8;

        MoveWindow(app->hwnd_calc_group, client_x, client_y, input_w, client_h, TRUE);

        /* Vehículo */
        MoveWindow(app->hwnd_calc_lbl_veh, client_x + 14, cur_y + 2, lbl_w, row_h, TRUE);
        MoveWindow(app->hwnd_calc_combo_veh, client_x + 14 + lbl_w, cur_y, edit_w, 200, TRUE);
        cur_y += row_h + row_gap;

        /* Distancia */
        MoveWindow(app->hwnd_calc_lbl_dist, client_x + 14, cur_y + 2, lbl_w, row_h, TRUE);
        MoveWindow(app->hwnd_calc_edit_dist, client_x + 14 + lbl_w, cur_y, edit_w, row_h, TRUE);
        cur_y += row_h + row_gap;

        /* Porcentaje ciudad */
        MoveWindow(app->hwnd_calc_lbl_city, client_x + 14, cur_y + 2, lbl_w, row_h, TRUE);
        MoveWindow(app->hwnd_calc_edit_city, client_x + 14 + lbl_w, cur_y, edit_w, row_h, TRUE);
        cur_y += row_h + row_gap;

        /* Porcentaje carretera */
        MoveWindow(app->hwnd_calc_lbl_hwy, client_x + 14, cur_y + 2, lbl_w, row_h, TRUE);
        MoveWindow(app->hwnd_calc_edit_hwy, client_x + 14 + lbl_w, cur_y, edit_w, row_h, TRUE);
        cur_y += row_h + row_gap;

        /* Precio combustible */
        MoveWindow(app->hwnd_calc_lbl_price, client_x + 14, cur_y + 2, lbl_w, row_h, TRUE);
        MoveWindow(app->hwnd_calc_edit_price, client_x + 14 + lbl_w, cur_y, edit_w, row_h, TRUE);
        cur_y += row_h + row_gap + 6;

        /* Botón Calcular */
        MoveWindow(app->hwnd_calc_btn_compute, client_x + 14 + lbl_w, cur_y, edit_w, 32, TRUE);

        /* Panel derecho de resultado */
        MoveWindow(app->hwnd_calc_lbl_result, res_x, client_y, res_w, 18, TRUE);
        MoveWindow(app->hwnd_calc_edit_result, res_x, client_y + 20, res_w, client_h - 20, TRUE);
    }
}

static void create_controls(HWND hwnd, AppState *app)
{
    HINSTANCE hinst = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);
    TCITEMW tie;
    LVCOLUMNW lvc;
    int i;

    /* Fuentes */
    app->font_ui = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    if (app->font_ui == NULL) {
        app->font_ui = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }

    app->font_mono = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
    if (app->font_mono == NULL) {
        app->font_mono = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
    }

    /* Tab Control */
    app->hwnd_tab = CreateWindowExW(0, WC_TABCONTROLW, L"",
                                    WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                    0, 0, 100, 100, hwnd, (HMENU)ID_TAB_CONTROL, hinst, NULL);
    set_ctrl_font(app->hwnd_tab, app->font_ui);

    memset(&tie, 0, sizeof(tie));
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)L"Vehiculos";
    TabCtrl_InsertItem(app->hwnd_tab, 0, &tie);
    tie.pszText = (LPWSTR)L"Calcular viaje";
    TabCtrl_InsertItem(app->hwnd_tab, 1, &tie);

    /* --- TAB 0: VEHICULOS --- */
    app->hwnd_lbl_list = CreateWindowExW(0, L"STATIC", L"Catalogo de vehiculos registrados:",
                                         WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_lbl_list, app->font_ui);

    app->hwnd_list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                     WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                     0, 0, 10, 10, hwnd, (HMENU)ID_LV_VEHICLES, hinst, NULL);
    set_ctrl_font(app->hwnd_list, app->font_ui);
    SendMessageW(app->hwnd_list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                 LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    memset(&lvc, 0, sizeof(lvc));
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvc.pszText = (LPWSTR)L"Nombre";
    lvc.cx = 160;
    SendMessageW(app->hwnd_list, LVM_INSERTCOLUMNW, 0, (LPARAM)&lvc);

    lvc.pszText = (LPWSTR)L"Cons. ciudad (km/L)";
    lvc.cx = 125;
    SendMessageW(app->hwnd_list, LVM_INSERTCOLUMNW, 1, (LPARAM)&lvc);

    lvc.pszText = (LPWSTR)L"Cons. carretera (km/L)";
    lvc.cx = 135;
    SendMessageW(app->hwnd_list, LVM_INSERTCOLUMNW, 2, (LPARAM)&lvc);

    app->hwnd_form_group = CreateWindowExW(0, L"BUTTON", L" Datos del vehiculo ",
                                           WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                           0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_form_group, app->font_ui);

    for (i = 0; i < NUM_FORM_FIELDS; i++) {
        app->hwnd_lbl[i] = CreateWindowExW(0, L"STATIC", g_field_labels[i],
                                           WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, hwnd, NULL, hinst, NULL);
        set_ctrl_font(app->hwnd_lbl[i], app->font_ui);

        app->hwnd_edit[i] = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                            0, 0, 10, 10, hwnd, (HMENU)(INT_PTR)(ID_EDIT_FORM_BASE + i), hinst, NULL);
        set_ctrl_font(app->hwnd_edit[i], app->font_ui);
    }

    app->hwnd_btn_new = CreateWindowExW(0, L"BUTTON", L"Nuevo",
                                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                        0, 0, 10, 10, hwnd, (HMENU)ID_BTN_NEW, hinst, NULL);
    set_ctrl_font(app->hwnd_btn_new, app->font_ui);

    app->hwnd_btn_save = CreateWindowExW(0, L"BUTTON", L"Guardar",
                                         WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                         0, 0, 10, 10, hwnd, (HMENU)ID_BTN_SAVE, hinst, NULL);
    set_ctrl_font(app->hwnd_btn_save, app->font_ui);

    app->hwnd_btn_delete = CreateWindowExW(0, L"BUTTON", L"Eliminar",
                                           WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                           0, 0, 10, 10, hwnd, (HMENU)ID_BTN_DELETE, hinst, NULL);
    set_ctrl_font(app->hwnd_btn_delete, app->font_ui);

    /* --- TAB 1: CALCULAR VIAJE --- */
    app->hwnd_calc_group = CreateWindowExW(0, L"BUTTON", L" Parametros del viaje ",
                                           WS_CHILD | BS_GROUPBOX,
                                           0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_group, app->font_ui);

    app->hwnd_calc_lbl_veh = CreateWindowExW(0, L"STATIC", L"Vehiculo:",
                                             WS_CHILD, 0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_lbl_veh, app->font_ui);

    app->hwnd_calc_combo_veh = CreateWindowExW(0, L"COMBOBOX", L"",
                                               WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                                               0, 0, 10, 10, hwnd, (HMENU)ID_COMBO_CALC_VEHICLE, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_combo_veh, app->font_ui);

    app->hwnd_calc_lbl_dist = CreateWindowExW(0, L"STATIC", L"Distancia total (km):",
                                              WS_CHILD, 0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_lbl_dist, app->font_ui);

    app->hwnd_calc_edit_dist = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"100.00",
                                               WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
                                               0, 0, 10, 10, hwnd, (HMENU)ID_EDIT_CALC_DIST, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_edit_dist, app->font_ui);

    app->hwnd_calc_lbl_city = CreateWindowExW(0, L"STATIC", L"Porcentaje urbano (0-100):",
                                              WS_CHILD, 0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_lbl_city, app->font_ui);

    app->hwnd_calc_edit_city = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"60.00",
                                               WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
                                               0, 0, 10, 10, hwnd, (HMENU)ID_EDIT_CALC_CITY_PCT, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_edit_city, app->font_ui);

    app->hwnd_calc_lbl_hwy = CreateWindowExW(0, L"STATIC", L"Porcentaje carretera (0-100):",
                                             WS_CHILD, 0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_lbl_hwy, app->font_ui);

    app->hwnd_calc_edit_hwy = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"40.00",
                                              WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
                                              0, 0, 10, 10, hwnd, (HMENU)ID_EDIT_CALC_HWY_PCT, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_edit_hwy, app->font_ui);

    app->hwnd_calc_lbl_price = CreateWindowExW(0, L"STATIC", L"Precio combustible (DOP/L):",
                                               WS_CHILD, 0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_lbl_price, app->font_ui);

    app->hwnd_calc_edit_price = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"100.00",
                                                WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
                                                0, 0, 10, 10, hwnd, (HMENU)ID_EDIT_CALC_PRICE, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_edit_price, app->font_ui);

    app->hwnd_calc_btn_compute = CreateWindowExW(0, L"BUTTON", L"Calcular costo del viaje",
                                                 WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                                 0, 0, 10, 10, hwnd, (HMENU)ID_BTN_CALCULATE, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_btn_compute, app->font_ui);

    app->hwnd_calc_lbl_result = CreateWindowExW(0, L"STATIC", L"Resultado y desglose del costo:",
                                                WS_CHILD, 0, 0, 10, 10, hwnd, NULL, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_lbl_result, app->font_ui);

    app->hwnd_calc_edit_result = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                                 WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                                 0, 0, 10, 10, hwnd, (HMENU)ID_EDIT_CALC_RESULT, hinst, NULL);
    set_ctrl_font(app->hwnd_calc_edit_result, app->font_mono);

    update_tab_visibility(app);
}

static LRESULT CALLBACK main_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    AppState *app = (AppState *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCTW *cs = (CREATESTRUCTW *)lparam;
        app = (AppState *)cs->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)app);
        app->hwnd_main = hwnd;
        create_controls(hwnd, app);
        refresh_list_view(app);
        return 0;
    }

    case WM_SIZE: {
        int width = LOWORD(lparam);
        int height = HIWORD(lparam);
        if (app != NULL) {
            layout_controls(app, width, height);
        }
        return 0;
    }

    case WM_GETMINMAXINFO: {
        MINMAXINFO *mmi = (MINMAXINFO *)lparam;
        mmi->ptMinTrackSize.x = 820;
        mmi->ptMinTrackSize.y = 560;
        return 0;
    }

    case WM_NOTIFY: {
        NMHDR *hdr = (NMHDR *)lparam;
        if (app == NULL) break;

        if (hdr->idFrom == ID_TAB_CONTROL && hdr->code == TCN_SELCHANGE) {
            app->active_tab = TabCtrl_GetCurSel(app->hwnd_tab);
            update_tab_visibility(app);
            if (app->active_tab == 1) {
                populate_calc_combo(app);
            }
            InvalidateRect(app->hwnd_main, NULL, TRUE);
            UpdateWindow(app->hwnd_main);
            return 0;
        }

        if (hdr->idFrom == ID_LV_VEHICLES && hdr->code == LVN_ITEMCHANGED) {
            NMLISTVIEW *nmlv = (NMLISTVIEW *)lparam;
            if (!app->updating_selection && (nmlv->uNewState & LVIS_SELECTED)) {
                app->selected_index = nmlv->iItem;
                load_vehicle_to_form(app, app->selected_index);
            }
            return 0;
        }
        break;
    }

    case WM_COMMAND: {
        int ctrl_id = LOWORD(wparam);
        if (app == NULL) break;

        switch (ctrl_id) {
        case ID_BTN_NEW:
            on_btn_new(app);
            return 0;
        case ID_BTN_SAVE:
            on_btn_save(app);
            return 0;
        case ID_BTN_DELETE:
            on_btn_delete(app);
            return 0;
        case ID_BTN_CALCULATE:
            on_btn_calculate(app);
            return 0;
        default:
            break;
        }
        break;
    }

    case WM_DESTROY: {
        if (app != NULL) {
            if (app->font_ui) DeleteObject(app->font_ui);
            if (app->font_mono) DeleteObject(app->font_mono);
        }
        PostQuitMessage(0);
        return 0;
    }

    default:
        break;
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE prev_instance, LPWSTR cmd_line, int show_cmd)
{
    INITCOMMONCONTROLSEX icc;
    WNDCLASSEXW wc;
    HWND hwnd;
    MSG msg;
    AppState app;
    char error[256];
    int loaded;

    (void)prev_instance;
    (void)cmd_line;

    memset(&app, 0, sizeof(app));
    app.selected_index = -1;
    app.active_tab = 0;

    /* Inicializar Common Controls v6 */
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    /* Cargar almacenamiento de vehiculos */
    loaded = storage_load(STORAGE_DEFAULT_PATH, &app.store, error, sizeof(error));
    if (loaded < 0) {
        wchar_t werr[512];
        utf8_to_wide(error[0] != '\0' ? error : "No se pudo leer el archivo de vehiculos.",
                     werr, sizeof(werr) / sizeof(werr[0]));
        MessageBoxW(NULL, werr, L"Error al cargar vehiculos", MB_ICONERROR | MB_OK);
        return 1;
    }
    if (loaded > 0) {
        wchar_t werr[512];
        utf8_to_wide(error[0] != '\0' ? error : "Se omitieron lineas invalidas del archivo.",
                     werr, sizeof(werr) / sizeof(werr[0]));
        MessageBoxW(NULL, werr, L"Aviso de carga", MB_ICONWARNING | MB_OK);
    }

    if (app.store.count > 0) {
        app.selected_index = 0;
    }

    /* Registrar clase de ventana */
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = main_wnd_proc;
    wc.hInstance = hinstance;
    wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"CarCostMainWindowClass";

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Error al registrar la clase de ventana.", L"Error fatal", MB_ICONERROR | MB_OK);
        return 1;
    }

    /* Crear ventana principal centrada */
    {
        int win_w = 960;
        int win_h = 640;
        int screen_w = GetSystemMetrics(SM_CXSCREEN);
        int screen_h = GetSystemMetrics(SM_CYSCREEN);
        int win_x = (screen_w > win_w) ? (screen_w - win_w) / 2 : CW_USEDEFAULT;
        int win_y = (screen_h > win_h) ? (screen_h - win_h) / 2 : CW_USEDEFAULT;

        hwnd = CreateWindowExW(WS_EX_WINDOWEDGE,
                               L"CarCostMainWindowClass",
                               L"Car Cost - Gestion y Calculo de Viajes",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                               win_x, win_y, win_w, win_h,
                               NULL, NULL, hinstance, &app);
    }

    if (hwnd == NULL) {
        MessageBoxW(NULL, L"Error al crear la ventana principal.", L"Error fatal", MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(hwnd, show_cmd);
    UpdateWindow(hwnd);

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageW(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return (int)msg.wParam;
}
