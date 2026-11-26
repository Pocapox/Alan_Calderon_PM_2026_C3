# Tasks

## 1. Estructura del proyecto y configuración de compilación

- [x] 1.1 Crear la estructura de directorios (`include/`, `src/`, `third_party/cJSON/`, `tests/`, `data/`) y agregar la librería `cJSON` (`cJSON.h` y `cJSON.c`) en `third_party/cJSON/`; verificar que los archivos existan en sus rutas.
- [x] 1.2 Crear `Makefile` / `CMakeLists.txt` (compatible con `gcc` MinGW-w64 y enlace `-lcomctl32 -lgdi32 -mwindows` para la GUI y compilación de pruebas de consola); verificar que el comando de compilación base reconozca la configuración sin errores.

## 2. Modelos de datos, listas enlazadas y memoria dinámica

- [x] 2.1 Definir las estructuras en `include/models.h` (`VehiculoNode`, `ClienteNode`, `MesPagoNode`, `AbonoNode`, `PrestamoNode` y el contenedor `SistemaPrestamos`) y declarar las operaciones en `include/linked_list.h`; verificar compilación de cabeceras.
- [x] 2.2 Implementar en `src/linked_list.c` las operaciones con memoria dinámica (`malloc`/`free`) para vehículos (agregar, buscar por ID, eliminar), clientes (agregar sin duplicar documento, buscar por ID/documento/nombre parcial, eliminar) y liberación completa de todas las listas enlazadas; verificar con pruebas unitarias en `tests/test_core.c` compiladas y ejecutadas con `gcc` que todas las aserciones de alta, búsqueda, borrado y liberación pasen.

## 3. Lógica de préstamos, pagos mensuales y abonos

- [x] 3.1 Implementar en `include/loan_service.h` y `src/loan_service.c` la creación de préstamos con tasa anual configurable (cuota fija mensual mediante amortización estándar para tasa > 0 y división directa para tasa 0) y la generación de la lista enlazada de meses inicialmente marcados como no pagados; verificar con pruebas unitarias en `tests/test_core.c` los cálculos de cuota fija y generación de meses.
- [x] 3.2 Implementar en `src/loan_service.c` la gestión de pagos mensuales (marcar mes pagado/no pagado y registrar fecha), el registro de pagos de abono con validación (`0 < abono <= saldoPendiente`) y el recálculo determinístico del saldo pendiente; verificar con pruebas unitarias en `tests/test_core.c` que los pagos mensuales y abonos actualicen correctamente el saldo y el estado de los meses.

## 4. Persistencia en archivos JSON

- [x] 4.1 Implementar en `include/json_store.h` y `src/json_store.c` la serialización y deserialización completa del `SistemaPrestamos` (clientes, vehículos, préstamos, lista de meses y abonos) hacia/desde `data/prestamos.json` usando `cJSON`; verificar con pruebas unitarias en `tests/test_core.c` que guardar y volver a cargar reconstruya exactamente las listas enlazadas, estados de meses, abonos y saldos.

## 5. Generación de reportes de estado de pago

- [x] 5.1 Implementar en `include/report.h` y `src/report.c` la generación de reportes de estado de pago en memoria (cadena formateada con cliente, vehículo, resumen financiero, lista de meses pagados/no pagados, abonos y saldo pendiente) y su exportación a archivo `.txt`; verificar con pruebas unitarias en `tests/test_core.c` que el reporte incluya todos los campos y se exporte correctamente a disco.

## 6. Interfaz gráfica de escritorio (Win32 GUI)

- [x] 6.1 Implementar la ventana principal y navegación por pestañas (`Clientes`, `Vehículos`, `Préstamos y Pagos`, `Reportes`) en `include/gui.h`, `src/gui.c` y `src/main.c`, cargando el JSON al iniciar y guardando al modificar/cerrar; verificar que el ejecutable `prestamos.exe` compile sin errores ni warnings críticos con `gcc`.
- [x] 6.2 Implementar en `src/gui.c` las pestañas de **Vehículos** (agregar marca, modelo, año y eliminar) y **Clientes** (agregar, buscar y borrar con validaciones y mensajes claros); verificar compilación y prueba funcional de alta, búsqueda y borrado en la interfaz.
- [x] 6.3 Implementar en `src/gui.c` la pestaña de **Préstamos y Pagos** (crear préstamo con tasa configurable, visualizar saldo pendiente y progreso, tabla/lista de meses pagados y no pagados con botón para marcar/desmarcar pago mensual, y formulario para registrar pagos de abono) y la pestaña de **Reportes** (generar vista previa del reporte de estado de pago y exportar a archivo); verificar compilación y flujo completo de extremo a extremo.

## 7. Validación integral

- [x] 7.1 Ejecutar la suite completa de pruebas unitarias (`tests/test_core.c`) y compilar la aplicación GUI final (`prestamos.exe`), verificando que todas las pruebas pasen y que la persistencia JSON funcione al reiniciar la aplicación.
