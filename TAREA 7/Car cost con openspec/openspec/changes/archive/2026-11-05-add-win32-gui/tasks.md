# Tasks

## 1. Núcleo compartido

- [x] 1.1 Mover `input_parse_double`, `input_parse_int` e `input_trim` a `src/parse.c/h`, dejar en `input.c` solo la lectura de `stdin` y actualizar includes. Verificación: `make test` sigue reportando 0 fallos.
- [x] 1.2 Añadir `src/utf8.c/h` con conversión UTF-8/UTF-16 y comprobación de tamaño de buffer. Verificación: pruebas de ida y vuelta con "Camión de José" y con buffer insuficiente pasan en `make test`.
- [x] 1.3 Subir `VEHICLE_NAME_MAX` a 128 bytes y ajustar la validación de longitud. Verificación: prueba que guarda y recupera un nombre UTF-8 largo y rechaza el que excede el límite.
- [x] 1.4 Configurar la consola del CLI en UTF-8 (`SetConsoleOutputCP` y `SetConsoleCP`). Verificación: un `vehicles.dat` con "Camión de José" se lista con el nombre correcto en `carcost.exe`.

## 2. Build y documentación de la GUI

- [x] 2.1 Añadir `src/gui.rc` con manifest de Common Controls v6 y DPI, y el target `gui` al `Makefile` con `windres`, `-mwindows -municode` y librerías Win32. Verificación: `make gui` produce `carcost-gui.exe` sin errores.
- [x] 2.2 Actualizar `AGENTS.md` con el layout de la GUI, `make gui`, `.\carcost-gui.exe` y la nota de datos compartidos. Verificación: cada comando documentado se ejecuta tal como está escrito.
- [x] 2.3 Verificar que el manifest se enlazó y que el `ListView` usa el estilo visual v6 al abrir la ventana. Verificación: la ventana se ve con controles modernos y sin mensajes de error de recursos.

## 3. Esqueleto de la ventana

- [x] 3.1 Implementar `wWinMain`, registro de clase, ventana principal, `TabControl`, fuente "Segoe UI" y tamaño mínimo. Verificación: `carcost-gui.exe` abre una ventana con las pestañas "Vehiculos" y "Calcular viaje", y cierra sin errores.
- [x] 3.2 Implementar `WM_SIZE` y el cambio de pestaña para recolocar y mostrar los controles correspondientes. Verificación: redimensionar y alternar pestañas mantiene los controles alineados y utilizables.

## 4. Pestaña Vehículos

- [x] 4.1 Crear el `ListView` con columnas Nombre, Consumo urbano y Consumo carretera, y poblarlo desde `VehicleStore`. Verificación: con dos vehículos en `vehicles.dat` se muestran dos filas con sus valores; sin vehículos, la lista queda vacía.
- [x] 4.2 Crear el formulario de 13 campos y cargar en él el vehículo seleccionado, convirtiendo UTF-8 a UTF-16. Verificación: seleccionar "Camión de José" muestra el nombre con acentos y todos sus parámetros.
- [x] 4.3 Implementar Nuevo y Guardar con parseo, validación, alta/edición y `storage_save`. Verificación: crear un vehículo válido y editar su seguro actualiza la lista y `vehicles.dat`.
- [x] 4.4 Mostrar diálogos de error para campos inválidos, nombre duplicado y fallo de guardado, sin alterar el almacén. Verificación: valor final mayor que el inicial y nombre repetido muestran error y conservan los datos previos.
- [x] 4.5 Implementar Eliminar con confirmación y persistencia. Verificación: confirmar borra el vehículo de la lista y del archivo; cancelar lo conserva.

## 5. Pestaña Calcular viaje

- [x] 5.1 Poblar el `ComboBox` con los vehículos y capturar distancia, porcentaje urbano, porcentaje carretera y precio por litro. Verificación: el combo lista los mismos vehículos que la pestaña anterior y acepta valores con coma o punto.
- [x] 5.2 Invocar `calculator_compute` y mostrar costo total, costo por km y desglose en un `Edit` de solo lectura. Verificación: 100 km, 60/40 y 100 DOP/L muestran 2683.33 DOP y 26.83 DOP/km.
- [x] 5.3 Mostrar diálogos para porcentajes que no suman 100, distancia cero o precio cero, y limpiar el resultado anterior. Verificación: cada entrada inválida muestra su error y no deja un resultado obsoleto visible.

## 6. Integración y verificación

- [x] 6.1 Probar el archivo compartido: crear en la GUI y listar en el CLI, y crear en el CLI y ver en la GUI tras reiniciar. Verificación: ambos ejecutables ven el mismo estado en `vehicles.dat`.
- [x] 6.2 Probar los errores de arranque: cabecera inválida muestra diálogo y cierra; línea inválida muestra aviso y carga el resto. Verificación: los dos escenarios se comportan como indican las specs y el archivo no se sobrescribe.
- [x] 6.3 Ejecutar la verificación completa: `make clean`, `make`, `make gui`, `make test`. Verificación: los cuatro comandos terminan con código 0 y sin advertencias.
- [x] 6.4 Recorrer la lista manual de la GUI: abrir, crear, editar, eliminar, calcular, Unicode, redimensionar y cambiar de pestaña. Verificación: todos los pasos coinciden con las specs y no aparecen cuelgues.

## Workflow follow-up

- Archivar primero `add-vehicle-trip-cost-calculator` para promover las specs base si aún no está archivado.
- Validar este cambio con `openspec validate add-win32-gui --strict`.
- Archivar `add-win32-gui` cuando la verificación manual esté completa.
