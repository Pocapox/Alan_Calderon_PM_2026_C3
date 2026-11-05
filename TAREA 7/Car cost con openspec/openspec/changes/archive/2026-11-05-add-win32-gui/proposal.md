# Proposal

## Why

La aplicación funciona, pero obliga a recordar un menú numerado y a escribir
cada campo por consola. Una ventana nativa de Windows hace visibles los datos del
vehículo, reduce errores de captura y permite comparar el resultado de un viaje
sin salir de la aplicación, manteniendo el mismo núcleo de cálculo.

## What Changes

- Nuevo ejecutable `carcost-gui.exe` con interfaz Win32 nativa, que convive con
  el `carcost.exe` de consola.
- Ventana principal con dos pestañas: **Vehículos** y **Calcular viaje**.
- Pestaña Vehículos: lista de vehículos, formulario con los 13 parámetros y
  acciones Nuevo, Guardar y Eliminar con confirmación.
- Pestaña Calcular viaje: selección de vehículo, distancia, porcentajes
  urbano/carretera y precio del combustible; muestra costo total, costo por km y
  desglose por rubro.
- Validación con los mismos mensajes del núcleo, mostrados en diálogos nativos.
- Soporte Unicode: nombres con acentos y `ñ` se capturan, guardan como UTF-8 y se
  vuelven a mostrar correctamente; la consola del CLI se configura en UTF-8.
- Extracción de los helpers de parseo a un módulo compartido y adición de
  utilidades de conversión UTF-8/UTF-16.
- Build nuevo `make gui` con `windres`, manifest de Common Controls v6 y
  enlaces `-municode -mwindows`.
- Sin cambios en las fórmulas de cálculo, en el formato de `vehicles.dat` ni en
  el comportamiento del CLI más allá de la salida UTF-8.

## Capabilities

### New Capabilities

- `desktop-gui`: ventana Win32, lista y formulario de vehículos, cálculo de
  viajes, diálogos de validación y soporte de texto Unicode en la interfaz.

### Modified Capabilities

- Ninguna. La GUI consume el comportamiento actual de gestión de vehículos y
  cálculo de viajes sin cambiar sus requisitos.

## Impact

- Archivos nuevos: `src/gui_main.c`, `src/gui.rc`, `src/parse.c/h`, `src/utf8.c/h`.
- Archivos modificados: `Makefile` (target `gui` y recursos), `src/input.c`
  (usa `parse`), `src/main.c` (consola UTF-8), `AGENTS.md` (comandos y layout).
- Sin dependencias externas: solo librerías del sistema Windows
  (`comctl32`, `uxtheme`, `gdi32`, `user32`).
- `vehicles.dat` sigue siendo el único almacén, en el directorio de trabajo, y
  es compartido por ambos ejecutables.
- Los 88 checks actuales deben seguir pasando; se añaden pruebas para `parse` y
  `utf8`.
