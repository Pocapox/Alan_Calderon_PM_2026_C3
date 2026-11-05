# Design

## Context

El núcleo actual (`vehicle`, `calculator`, `storage`) no depende de la consola:
la interfaz vive en `main.c` y la lectura de `stdin` en `input.c`. Eso permite
añadir un segundo entrypoint sin duplicar la lógica de costos ni el formato de
`vehicles.dat`. Ver `proposal.md` para la motivación.

El entorno ya tiene Win32 disponible en MinGW-w64 (`windows.h`, `commctrl.h`,
`uxtheme.h`, `windres`, `libcomctl32`, `libuxtheme`). No hay GTK, SDL2, raylib ni
GLFW instalados. La ruta de trabajo contiene un espacio, así que las recetas del
`Makefile` usan rutas relativas.

## Goals / Non-Goals

**Goals:**

- GUI nativa, Unicode, sin dependencias externas y en un solo `.exe`.
- Reutilizar el núcleo y mantener intactos los 88 checks actuales.
- Compartir `vehicles.dat` con el CLI sin migrar datos.
- Aislar la lógica de UI para que el cálculo siga siendo testeable.

**Non-Goals:**

- Reemplazar el CLI, soporte multiplataforma o temas personalizados.
- Bloqueo de archivo entre CLI y GUI, o edición concurrente.
- Mover `vehicles.dat` a `%APPDATA%` o junto al ejecutable.
- Avisos de "cambios sin guardar" al cambiar de selección.
- Pruebas automatizadas de la interfaz Win32.

## Decisions

### Win32 nativo con Common Controls

Se usa la API Win32 directa: `TabControl`, `ListView`, `Edit`, `ComboBox`,
`Button` y `Static`. Alternativas descartadas: IUP (requiere DLLs), GTK (runtime
grande), raylib (look no nativo y controles de texto pobres) y Dear ImGui
(obliga a compilar C++). Win32 no añade instalación y produce un único `.exe`.

### Segundo ejecutable que convive con el CLI

Se añade `carcost-gui.exe` y se conserva `carcost.exe`. Alternativa descartada:
un solo ejecutable con `--cli`, que complica el `WinMain` y rompe la invocación
actual. El CLI sigue siendo la vía rápida y la suite de pruebas no cambia de
entrypoint.

### Dos pestañas

Pestaña "Vehiculos": `ListView` a la izquierda y formulario de 13 campos a la
derecha, con botones Nuevo, Guardar y Eliminar. Pestaña "Calcular viaje":
`ComboBox` de vehículos, cuatro entradas y un `Edit` multilínea de solo lectura
para el resultado. Alternativa descartada: ventana única con lista, formulario y
cálculo visibles a la vez; los 13 campos más el resultado la vuelven densa.

### Unicode en la UI y UTF-8 en el núcleo

La GUI usa `wWinMain`, `-municode`, `-DUNICODE` y controles `W`. El modelo sigue
guardando `char` en UTF-8. En el borde se convierte con
`WideCharToMultiByte(CP_UTF8, ...)` al guardar y `MultiByteToWideChar(CP_UTF8,
...)` al mostrar. `VEHICLE_NAME_MAX` sube de 64 a 128 bytes para que un nombre
con acentos no pierda longitud útil.

El CLI configura `SetConsoleOutputCP(CP_UTF8)` y `SetConsoleCP(CP_UTF8)` para
mostrar los mismos nombres. Alternativa descartada: ANSI puro; evita las
conversiones pero impide escribir "Camión" o "José" correctamente.

### Parseo compartido

`input_parse_double`, `input_parse_int` y `input_trim` se mueven a
`src/parse.c/h`. `input.c` queda como capa de `stdin` y la GUI llama directamente
a las funciones puras. Así se evita enlazar `getchar` en la GUI y se pueden
probar los parsers sin simular la consola. Alternativa descartada: duplicar el
parseo en la GUI, que ya provocó diferencias de comportamiento en el pasado.

### Estado y flujo de eventos

Una única `AppState` se engancha con `SetWindowLongPtr(hwnd, GWLP_USERDATA,
state)` y contiene el `VehicleStore`, el índice seleccionado, el modo alta/edición
y los `HWND` de los controles. `WndProc` delega en funciones pequeñas:

```
WM_CREATE   -> crear controles, fuente, cargar store
WM_NOTIFY   -> LVN_ITEMCHANGED: vehiculo -> formulario
WM_COMMAND  -> Nuevo / Guardar / Eliminar / Calcular
WM_SIZE     -> layout proporcional con tamano minimo
WM_DESTROY  -> PostQuitMessage
```

La ventana arranca en 960x640, es redimensionable y no baja de 820x560. El
layout se recalcula en `WM_SIZE`; no se usa ningún motor de layout externo.

### Persistencia y errores

La GUI llama a `storage_save(STORAGE_DEFAULT_PATH, ...)` después de cada alta,
edición o borrado. Si falla, revierte el cambio en memoria y muestra
`MessageBoxW` con el error, igual que hace el CLI. Al arrancar:

- cabecera inválida o error de lectura: diálogo de error y cierre sin guardar;
- líneas omitidas: diálogo de aviso y arranque con los vehículos válidos.

Todos los mensajes usan `MessageBoxW`; con `-mwindows` no hay consola donde
imprimir. Alternativa descartada: registrar errores en un archivo, que el usuario
no vería.

### Recursos, manifest y build

`src/gui.rc` incluye el manifest de Common Controls v6 y DPI awareness, más un
icono opcional. `windres` lo compila a COFF y se enlaza con el ejecutable. El
`Makefile` añade:

```
make gui    -> carcost-gui.exe
```

con `-mwindows -municode -lcomctl32 -luxtheme -lgdi32 -luser32`. `make test`
sigue compilando el runner con el núcleo, `parse` y `utf8`, sin enlazar Win32.
Alternativa descartada: CMake, que no está instalado; el `Makefile` se mantiene
compatible con GNU Make 3.81.

### Pruebas

- El núcleo conserva sus pruebas actuales.
- Se añaden pruebas de `parse` (coma/punto, texto sobrante, enteros) y de `utf8`
  (ida y vuelta con acentos y `ñ`, buffers insuficientes).
- La GUI se valida con una lista de comprobación manual: apertura, CRUD,
  cálculo, Unicode, errores y persistencia compartida con el CLI.
- No se añade automatización de UI; requeriría herramientas externas.

## Risks / Trade-offs

- **Boilerplate Win32** → `WndProc` delgado y funciones por área; IDs de control
  en un enum único.
- **Conversión Unicode defectuosa** → helpers centralizados con longitud
  explícita, comprobación de retorno y pruebas de ida y vuelta.
- **Nombres largos o truncados** → `VEHICLE_NAME_MAX` de 128 bytes y error visible
  si la conversión no cabe; nunca guardar un nombre truncado en silencio.
- **DPI y fuentes** → manifest con DPI awareness, fuente "Segoe UI" 9pt y prueba
  visual al 100% y 150%.
- **`-mwindows` oculta errores** → todo fallo pasa por `MessageBoxW`; prohibido
  `printf` en la GUI.
- **CLI y GUI escriben a la vez** → último en guardar gana; se documenta como
  limitación y no se implementa bloqueo.
- **`vehicles.dat` depende del directorio de trabajo** → se documenta ejecutar
  desde la raíz del proyecto o junto al ejecutable; un acceso directo con otro
  directorio crearía un archivo distinto.
- **Manifest ausente o mal enlazado** → la app funciona con controles clásicos;
  verificar visualmente que el `ListView` usa el estilo v6.

## Migration Plan

No hay migración de datos: los nombres ASCII existentes siguen siendo UTF-8
válido y el formato `carcost 1` no cambia. El CLI no se retira. Para revertir,
basta eliminar el target `gui`, `gui_main.c`, `gui.rc`, `parse.*` y `utf8.*`, y
restaurar `input.c`; el ejecutable y los datos del CLI quedan intactos.

## Open Questions

Ninguna pendiente que afecte specs, enfoque o desglose de tareas.
