#include "json_store.h"
#include "linked_list.h"
#include "loan_service.h"
#include "cJSON.h"

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_err(char *errBuf, int errSize, const char *msg) {
    if (errBuf && errSize > 0) {
        snprintf(errBuf, (size_t)errSize, "%s", msg ? msg : "");
    }
}

static void asegurar_directorio_padre(const char *ruta) {
    if (!ruta) return;
    char *copy = str_clone(ruta);
    if (!copy) return;
    for (char *p = copy; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char saved = *p;
            *p = '\0';
            if (copy[0] != '\0') {
                _mkdir(copy);
            }
            *p = saved;
        }
    }
    free(copy);
}

int json_guardar(const SistemaPrestamos *sys, const char *ruta, char *errBuf, int errSize) {
    if (!sys || !ruta) {
        set_err(errBuf, errSize, "Parametros invalidos para guardar JSON.");
        return 0;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        set_err(errBuf, errSize, "Error de memoria al construir objeto JSON.");
        return 0;
    }

    cJSON_AddNumberToObject(root, "nextVehiculoId", sys->nextVehiculoId);
    cJSON_AddNumberToObject(root, "nextClienteId", sys->nextClienteId);
    cJSON_AddNumberToObject(root, "nextPrestamoId", sys->nextPrestamoId);

    cJSON *arrVehiculos = cJSON_AddArrayToObject(root, "vehiculos");
    for (VehiculoNode *v = sys->vehiculos; v; v = v->next) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", v->id);
        cJSON_AddStringToObject(item, "marca", v->marca ? v->marca : "");
        cJSON_AddStringToObject(item, "modelo", v->modelo ? v->modelo : "");
        cJSON_AddNumberToObject(item, "anio", v->anio);
        cJSON_AddItemToArray(arrVehiculos, item);
    }

    cJSON *arrClientes = cJSON_AddArrayToObject(root, "clientes");
    for (ClienteNode *c = sys->clientes; c; c = c->next) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", c->id);
        cJSON_AddStringToObject(item, "nombre", c->nombre ? c->nombre : "");
        cJSON_AddStringToObject(item, "documento", c->documento ? c->documento : "");
        cJSON_AddStringToObject(item, "telefono", c->telefono ? c->telefono : "");
        cJSON_AddItemToArray(arrClientes, item);
    }

    cJSON *arrPrestamos = cJSON_AddArrayToObject(root, "prestamos");
    for (PrestamoNode *p = sys->prestamos; p; p = p->next) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", p->id);
        cJSON_AddNumberToObject(item, "clienteId", p->clienteId);
        cJSON_AddNumberToObject(item, "vehiculoId", p->vehiculoId);
        cJSON_AddNumberToObject(item, "montoFinanciado", p->montoFinanciado);
        cJSON_AddNumberToObject(item, "tasaAnual", p->tasaAnual);
        cJSON_AddNumberToObject(item, "plazoMeses", p->plazoMeses);
        cJSON_AddNumberToObject(item, "cuotaMensual", p->cuotaMensual);
        cJSON_AddNumberToObject(item, "saldoPendiente", p->saldoPendiente);
        cJSON_AddBoolToObject(item, "liquidado", p->liquidado != 0);
        cJSON_AddNumberToObject(item, "nextAbonoId", p->nextAbonoId);

        cJSON *arrMeses = cJSON_AddArrayToObject(item, "meses");
        for (MesPagoNode *m = p->meses; m; m = m->next) {
            cJSON *mItem = cJSON_CreateObject();
            cJSON_AddNumberToObject(mItem, "numeroMes", m->numeroMes);
            cJSON_AddNumberToObject(mItem, "montoCuota", m->montoCuota);
            cJSON_AddNumberToObject(mItem, "capitalCuota", m->capitalCuota);
            cJSON_AddNumberToObject(mItem, "interesCuota", m->interesCuota);
            cJSON_AddBoolToObject(mItem, "pagado", m->pagado != 0);
            cJSON_AddStringToObject(mItem, "fechaPago", m->fechaPago ? m->fechaPago : "");
            cJSON_AddItemToArray(arrMeses, mItem);
        }

        cJSON *arrAbonos = cJSON_AddArrayToObject(item, "abonos");
        for (AbonoNode *a = p->abonos; a; a = a->next) {
            cJSON *aItem = cJSON_CreateObject();
            cJSON_AddNumberToObject(aItem, "id", a->id);
            cJSON_AddNumberToObject(aItem, "monto", a->monto);
            cJSON_AddStringToObject(aItem, "fecha", a->fecha ? a->fecha : "");
            cJSON_AddItemToArray(arrAbonos, aItem);
        }

        cJSON_AddItemToArray(arrPrestamos, item);
    }

    char *jsonStr = cJSON_Print(root);
    cJSON_Delete(root);
    if (!jsonStr) {
        set_err(errBuf, errSize, "Error al serializar JSON.");
        return 0;
    }

    asegurar_directorio_padre(ruta);
    FILE *fp = fopen(ruta, "wb");
    if (!fp) {
        cJSON_free(jsonStr);
        set_err(errBuf, errSize, "No se pudo abrir el archivo JSON para escritura.");
        return 0;
    }

    size_t len = strlen(jsonStr);
    size_t written = fwrite(jsonStr, 1, len, fp);
    fclose(fp);
    cJSON_free(jsonStr);

    if (written != len) {
        set_err(errBuf, errSize, "Error al escribir el contenido completo en JSON.");
        return 0;
    }

    set_err(errBuf, errSize, "");
    return 1;
}

int json_cargar(SistemaPrestamos *sys, const char *ruta, char *errBuf, int errSize) {
    if (!sys || !ruta) {
        set_err(errBuf, errSize, "Parametros invalidos para cargar JSON.");
        return 0;
    }

    sistema_free(sys);
    sistema_init(sys);

    FILE *fp = fopen(ruta, "rb");
    if (!fp) {
        /* Archivo aún no existe: iniciar vacío sin error */
        set_err(errBuf, errSize, "");
        return 1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        set_err(errBuf, errSize, "Error al leer el tamano del archivo JSON.");
        return 0;
    }
    long size = ftell(fp);
    rewind(fp);

    if (size <= 0) {
        fclose(fp);
        set_err(errBuf, errSize, "");
        return 1;
    }

    char *buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) {
        fclose(fp);
        set_err(errBuf, errSize, "Error de memoria al leer archivo JSON.");
        return 0;
    }

    size_t readBytes = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);
    buffer[readBytes] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        set_err(errBuf, errSize, "El archivo JSON tiene un formato invalido.");
        return 0;
    }

    cJSON *nv = cJSON_GetObjectItemCaseSensitive(root, "nextVehiculoId");
    cJSON *nc = cJSON_GetObjectItemCaseSensitive(root, "nextClienteId");
    cJSON *np = cJSON_GetObjectItemCaseSensitive(root, "nextPrestamoId");
    if (cJSON_IsNumber(nv)) sys->nextVehiculoId = nv->valueint;
    if (cJSON_IsNumber(nc)) sys->nextClienteId = nc->valueint;
    if (cJSON_IsNumber(np)) sys->nextPrestamoId = np->valueint;

    cJSON *arrVehiculos = cJSON_GetObjectItemCaseSensitive(root, "vehiculos");
    cJSON *vItem = NULL;
    cJSON_ArrayForEach(vItem, arrVehiculos) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(vItem, "id");
        cJSON *marca = cJSON_GetObjectItemCaseSensitive(vItem, "marca");
        cJSON *modelo = cJSON_GetObjectItemCaseSensitive(vItem, "modelo");
        cJSON *anio = cJSON_GetObjectItemCaseSensitive(vItem, "anio");
        if (cJSON_IsNumber(id) && cJSON_IsString(marca) && cJSON_IsString(modelo) && cJSON_IsNumber(anio)) {
            vehiculo_agregar_con_id(sys, id->valueint, marca->valuestring, modelo->valuestring, anio->valueint);
        }
    }

    cJSON *arrClientes = cJSON_GetObjectItemCaseSensitive(root, "clientes");
    cJSON *cItem = NULL;
    cJSON_ArrayForEach(cItem, arrClientes) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(cItem, "id");
        cJSON *nombre = cJSON_GetObjectItemCaseSensitive(cItem, "nombre");
        cJSON *doc = cJSON_GetObjectItemCaseSensitive(cItem, "documento");
        cJSON *tel = cJSON_GetObjectItemCaseSensitive(cItem, "telefono");
        if (cJSON_IsNumber(id) && cJSON_IsString(nombre) && cJSON_IsString(doc) && cJSON_IsString(tel)) {
            cliente_agregar_con_id(sys, id->valueint, nombre->valuestring, doc->valuestring, tel->valuestring);
        }
    }

    cJSON *arrPrestamos = cJSON_GetObjectItemCaseSensitive(root, "prestamos");
    cJSON *pItem = NULL;
    PrestamoNode *pTail = NULL;
    cJSON_ArrayForEach(pItem, arrPrestamos) {
        PrestamoNode *p = (PrestamoNode *)malloc(sizeof(PrestamoNode));
        if (!p) continue;
        memset(p, 0, sizeof(PrestamoNode));

        cJSON *id = cJSON_GetObjectItemCaseSensitive(pItem, "id");
        cJSON *cid = cJSON_GetObjectItemCaseSensitive(pItem, "clienteId");
        cJSON *vid = cJSON_GetObjectItemCaseSensitive(pItem, "vehiculoId");
        cJSON *monto = cJSON_GetObjectItemCaseSensitive(pItem, "montoFinanciado");
        cJSON *tasa = cJSON_GetObjectItemCaseSensitive(pItem, "tasaAnual");
        cJSON *plazo = cJSON_GetObjectItemCaseSensitive(pItem, "plazoMeses");
        cJSON *cuota = cJSON_GetObjectItemCaseSensitive(pItem, "cuotaMensual");
        cJSON *nextAbono = cJSON_GetObjectItemCaseSensitive(pItem, "nextAbonoId");

        p->id = cJSON_IsNumber(id) ? id->valueint : sys->nextPrestamoId++;
        p->clienteId = cJSON_IsNumber(cid) ? cid->valueint : 0;
        p->vehiculoId = cJSON_IsNumber(vid) ? vid->valueint : 0;
        p->montoFinanciado = cJSON_IsNumber(monto) ? monto->valuedouble : 0.0;
        p->tasaAnual = cJSON_IsNumber(tasa) ? tasa->valuedouble : 0.0;
        p->plazoMeses = cJSON_IsNumber(plazo) ? plazo->valueint : 0;
        p->cuotaMensual = cJSON_IsNumber(cuota) ? cuota->valuedouble : 0.0;
        p->nextAbonoId = cJSON_IsNumber(nextAbono) ? nextAbono->valueint : 1;

        cJSON *arrMeses = cJSON_GetObjectItemCaseSensitive(pItem, "meses");
        cJSON *mItem = NULL;
        MesPagoNode *mTail = NULL;
        cJSON_ArrayForEach(mItem, arrMeses) {
            MesPagoNode *m = (MesPagoNode *)malloc(sizeof(MesPagoNode));
            if (!m) continue;
            cJSON *num = cJSON_GetObjectItemCaseSensitive(mItem, "numeroMes");
            cJSON *mc = cJSON_GetObjectItemCaseSensitive(mItem, "montoCuota");
            cJSON *cap = cJSON_GetObjectItemCaseSensitive(mItem, "capitalCuota");
            cJSON *intc = cJSON_GetObjectItemCaseSensitive(mItem, "interesCuota");
            cJSON *pag = cJSON_GetObjectItemCaseSensitive(mItem, "pagado");
            cJSON *fpStr = cJSON_GetObjectItemCaseSensitive(mItem, "fechaPago");

            m->numeroMes = cJSON_IsNumber(num) ? num->valueint : 0;
            m->montoCuota = cJSON_IsNumber(mc) ? mc->valuedouble : 0.0;
            m->capitalCuota = cJSON_IsNumber(cap) ? cap->valuedouble : 0.0;
            m->interesCuota = cJSON_IsNumber(intc) ? intc->valuedouble : 0.0;
            m->pagado = cJSON_IsBool(pag) ? cJSON_IsTrue(pag) : 0;
            m->fechaPago = str_clone(cJSON_IsString(fpStr) ? fpStr->valuestring : "");
            m->next = NULL;

            if (!p->meses) {
                p->meses = m;
                mTail = m;
            } else {
                mTail->next = m;
                mTail = m;
            }
        }

        cJSON *arrAbonos = cJSON_GetObjectItemCaseSensitive(pItem, "abonos");
        cJSON *aItem = NULL;
        AbonoNode *aTail = NULL;
        cJSON_ArrayForEach(aItem, arrAbonos) {
            AbonoNode *a = (AbonoNode *)malloc(sizeof(AbonoNode));
            if (!a) continue;
            cJSON *aid = cJSON_GetObjectItemCaseSensitive(aItem, "id");
            cJSON *am = cJSON_GetObjectItemCaseSensitive(aItem, "monto");
            cJSON *af = cJSON_GetObjectItemCaseSensitive(aItem, "fecha");

            a->id = cJSON_IsNumber(aid) ? aid->valueint : p->nextAbonoId++;
            a->monto = cJSON_IsNumber(am) ? am->valuedouble : 0.0;
            a->fecha = str_clone(cJSON_IsString(af) ? af->valuestring : "");
            a->next = NULL;

            if (a->id >= p->nextAbonoId) {
                p->nextAbonoId = a->id + 1;
            }
            if (!p->abonos) {
                p->abonos = a;
                aTail = a;
            } else {
                aTail->next = a;
                aTail = a;
            }
        }

        prestamo_recalcular_saldo(p);
        if (p->id >= sys->nextPrestamoId) {
            sys->nextPrestamoId = p->id + 1;
        }

        if (!sys->prestamos) {
            sys->prestamos = p;
            pTail = p;
        } else {
            pTail->next = p;
            pTail = p;
        }
    }

    cJSON_Delete(root);
    set_err(errBuf, errSize, "");
    return 1;
}
