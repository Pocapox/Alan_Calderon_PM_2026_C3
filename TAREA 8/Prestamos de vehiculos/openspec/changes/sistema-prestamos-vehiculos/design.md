# Design

## Context

El proyecto `Prestamos` inicia desde cero (`greenfield`). El entorno cuenta con `gcc` 16.2.0 (MinGW-w64 UCRT, WinLibs) y `cmake` en Windows x64. Las restricciones principales definidas para el cambio son:
- Lenguaje **C** con gestión explícita de **memoria dinámica** (`malloc`/`calloc`/`realloc`/`free`).
- Estructuras de datos principales implementadas mediante **listas enlazadas** (`struct` con punteros `next`).
- Persistencia en archivos **JSON**.
- **Interfaz gráfica de escritorio (GUI)** nativa en Windows.
- Modelo de préstamo con **tasa de interés anual configurable (cuota fija mensual)** y **pagos de abono que reducen el saldo pendiente**.

Ver `proposal.md` para la motivación y las capacidades del sistema.

## Goals / Non-Goals

**Goals:**
- Implementar una arquitectura modular en C11 separando dominio (listas enlazadas y lógica financiera), persistencia JSON y capa de presentación Win32 GUI.
- Garantizar cero fugas de memoria al liberar todas las listas enlazadas anidadas al borrar registros o cerrar la aplicación.
- Proveer una GUI funcional y responsiva usando únicamente APIs incluidas en MinGW-w64 (Win32 + Common Controls) para compilar sin dependencias externas del sistema.
- Mantener la integridad referencial en memoria y en JSON mediante IDs únicos entre clientes, vehículos y préstamos.

**Non-Goals:**
- Soporte multiplataforma (Linux/macOS) en la capa GUI; el diseño apunta a Windows x64.
- Base de datos relacional (SQLite/PostgreSQL); la persistencia es exclusivamente mediante archivo JSON.
- Autenticación multiusuario, roles o conexión en red.

## Decisions

### 1. Interfaz gráfica con Win32 API + Common Controls (`windows.h`, `commctrl.h`)
- **Decisión**: Usar Win32 API nativa en C (`CreateWindowEx`, `TabControl`, `ListView`, `Edit`, `Button`, `ComboBox`, `MessageBox`) enlazando `-lcomctl32 -lgdi32 -mwindows`.
- **Alternativas consideradas**: GTK3/GTK4 o Raylib/Nuklear. Se descartaron porque requieren instalar librerías externas y DLLs en el entorno Windows, mientras que Win32 API viene integrada en MinGW-w64 y produce un único ejecutable `.exe` autocontenido.

### 2. Persistencia JSON con `cJSON` integrado (vendored)
- **Decisión**: Incorporar `cJSON.h` y `cJSON.c` (licencia MIT) en `third_party/cJSON/` para parsear y serializar el archivo `data/prestamos.json`.
- **Alternativas consideradas**: Escribir un parser JSON manual desde cero. Se descartó por el alto riesgo de errores en escape de cadenas, formato numérico y mantenimiento frente a un estándar probado de 2 archivos en C puro.

### 3. Modelo de datos con listas enlazadas y memoria dinámica
- **Decisión**: Definir listas enlazadas explícitas para cada entidad:
  - `ListaVehiculos` -> nodos `VehiculoNode` (`id`, `marca`, `modelo`, `anio`, `next`).
  - `ListaClientes` -> nodos `ClienteNode` (`id`, `nombre`, `documento`, `telefono`, `next`).
  - `ListaPrestamos` -> nodos `PrestamoNode` (`id`, `clienteId`, `vehiculoId`, `montoFinanciado`, `tasaAnual`, `plazoMeses`, `cuotaMensual`, `saldoPendiente`, `meses` [lista enlazada `MesPagoNode`], `abonos` [lista enlazada `AbonoNode`], `next`).
- Cada nodo y sus cadenas dinámicas (o buffers acotados dentro del `struct` asignado con `malloc`) se administran mediante funciones `crear_*`, `agregar_*`, `buscar_*`, `eliminar_*` y `liberar_*`.

### 4. Cálculo financiero de cuota fija y efecto de los abonos
- **Decisión**:
  - Si `tasaAnual > 0`, la tasa mensual es $i = \frac{\text{tasaAnual}}{12 \times 100}$ y la cuota fija mensual es $C = P \cdot \frac{i (1+i)^n}{(1+i)^n - 1}$.
  - Si `tasaAnual == 0`, $C = \frac{P}{n}$.
  - El `saldoPendiente` inicia en el total del capital financiado (o monto total pendiente según el desglose capital/interés de cada cuota). Para mantener claridad contable con los abonos a capital y el marcado de meses: cada `MesPagoNode` guarda `numeroMes`, `montoCuota`, `capitalCuota`, `interesCuota`, `pagado` (`0`/`1`) y `fechaPago`. Al marcar un mes como pagado, se descuenta su `capitalCuota` del `saldoPendiente`; si se revierte a no pagado, se restaura. Al registrar un `Abono`, se descuenta directamente el monto del abono del `saldoPendiente` (limitado a `<= saldoPendiente`). Cuando `saldoPendiente <= 0.01`, el saldo se ajusta a `0.00` y el préstamo queda liquidado.

### 5. Estructura de directorios del proyecto
- `include/`: cabeceras (`models.h`, `linked_list.h`, `loan_service.h`, `json_store.h`, `report.h`, `gui.h`).
- `src/`: implementaciones en C (`main.c`, `linked_list.c`, `loan_service.c`, `json_store.c`, `report.c`, `gui.c`).
- `third_party/cJSON/`: `cJSON.h`, `cJSON.c`.
- `tests/`: pruebas unitarias en C para listas enlazadas, cálculo de cuotas/abonos y serialización JSON.
- `data/`: almacenamiento por defecto `prestamos.json`.
- `CMakeLists.txt` y `Makefile` (o script de compilación `gcc`).

## Risks / Trade-offs

- **Complejidad de código Win32 en C** → Se mitigará encapsulando cada pestaña (Clientes, Vehículos, Préstamos/Pagos, Reportes) en funciones dedicadas dentro de `gui.c` y usando controles estándar (`ListView` con `LVS_REPORT`) para mostrar tablas de forma limpia.
- **Desincronización entre lista de meses y saldo tras abonos extraordinarios** → Se mitigará manteniendo el `saldoPendiente` como fuente de verdad del capital restante, recalculándolo de forma determinística a partir de `montoFinanciado - sum(capital de meses pagados) - sum(abonos)` (con piso en `0.00`), de modo que revertir un mes o cargar desde JSON siempre sea consistente.
- **Manejo de memoria en C** → Se mitigará creando funciones `liberar_*` dedicadas para cada lista enlazada y ejecutando pruebas unitarias de ciclo completo (crear, buscar, borrar, guardar, cargar, liberar).
