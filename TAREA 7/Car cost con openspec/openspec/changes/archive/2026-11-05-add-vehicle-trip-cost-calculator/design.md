# Design

## Context

Proyecto C nuevo sin código previo. Ver `proposal.md` para la motivación. El
entorno es Windows con PowerShell, sin GCC instalado, sin CMake, con GNU Make
3.81 disponible y con una ruta de trabajo que contiene un espacio.

Las decisiones de unidades ya están fijadas: DOP, precio por litro y consumo en
km/L. El costo de limpieza se modela como costo por lavado por frecuencia anual.

## Goals / Non-Goals

**Goals:**

- Cálculo determinista y testeable, separado de la interfaz y del almacenamiento.
- CRUD completo con persistencia simple, sin dependencias externas.
- Manejo explícito de errores de entrada sin abortar el programa.
- Compilar con un solo comando en Windows usando GCC/MinGW-w64.

**Non-Goals:**

- Interfaz gráfica, red, multiusuario o concurrencia.
- Múltiples monedas o unidades configurables por vehículo.
- Depreciación no lineal, reglas fiscales por país ni precios de combustible en línea.
- Historial de viajes; cada cálculo es efímero.

## Decisions

### Estructura modular en C

Cuatro módulos con encabezados: `vehicle` (modelo y validación), `storage`
(carga/guardado), `calculator` (funciones puras de costo) y `ui`/`main` (menú y
entrada/salida). Alternativa considerada: un solo `main.c`; se descarta porque
hace imposible probar la fórmula sin simular la consola.

### Fórmula central

```
depreciacion_ano = (valor_inicial - valor_final) / anos_uso
limpieza_ano     = costo_lavado * lavados_ano
fijo_ano         = seguro + impuestos + depreciacion_ano + limpieza_ano
fijo_km          = fijo_ano / km_ano

km_ciudad    = D * p_ciudad / 100
km_carretera = D * p_carretera / 100
combustible  = km_ciudad / rend_ciudad * precio_litro
             + km_carretera / rend_carretera * precio_litro
variable     = (costo_neumaticos_km + costo_mantenimiento_km) * D

costo_viaje = combustible + variable + fijo_km * D
costo_km    = costo_viaje / D
```

El reparto de fijos usa el kilometraje promedio anual del vehículo, no la
distancia del viaje. Alternativas descartadas: prorratear por días de viaje
(hace que el costo/km dependa de la duración) y omitir los fijos (subestima el
costo real).

### El combustible no se promedia

Con consumos en km/L, el consumo combinado se obtiene sumando litros por tramo
(`km / km_por_litro`), no promediando los rendimientos. La fórmula del diseño ya
refleja esto; las pruebas deben cubrir un caso con rendimientos muy distintos.

### Precio del combustible como entrada del cálculo

El precio por litro se pide en cada cálculo, no se guarda en el vehículo, porque
cambia con frecuencia. Alternativa descartada: almacenarlo por vehículo y
obligar a editarlo antes de cada cálculo.

### Persistencia en texto delimitado

Archivo `vehicles.dat` en el directorio de trabajo, con una línea de cabecera de
versión y un vehículo por línea en formato separado por `|`. Se prohíbe el
carácter `|` y los saltos de línea en el nombre. Guardado atómico: escribir en
`vehicles.tmp` y renombrar sobre el archivo final.

Alternativas descartadas: CSV con comillas (más código de parseo), JSON a mano
(frágil sin librería) y SQLite (dependencia externa no instalada). El formato es
deliberadamente simple y visible para poder inspeccionarlo y corregirlo.

### Números y precisión

`double` para todos los montos y consumos. Se muestra con dos decimales, pero los
cálculos y comparaciones usan el valor completo con una tolerancia de `1e-9` en
las pruebas. Alternativa descartada: centavos enteros; no elimina el problema
porque consumos y porcentajes también son fraccionarios.

### Entrada tolerante al separador decimal

Se normaliza `,` a `.` antes de convertir con `strtod`, porque el usuario
hispanohablante puede escribir `12,5`. Se rechaza texto sobrante después del
número. El programa no depende de la configuración regional de Windows.

### Build con Makefile y GCC

Se usa un `Makefile` compatible con GNU Make 3.81 (el instalado) y reglas
simples que invocan `gcc -std=c11 -Wall -Wextra -Wpedantic`. Objetivos:
`all`, `test`, `clean`. Alternativas descartadas: CMake (no instalado) y scripts
de PowerShell (menos convencionales y no verificables con `make`). El espacio en
la ruta no afecta las recetas porque dentro del `Makefile` se usan rutas
relativas sin espacios.

### Pruebas sin framework externo

Un binario `tests/run_tests.c` con `assert` y una función auxiliar de comparación
con tolerancia. Se compila y ejecuta con `make test`. Cubre la fórmula completa,
cada caso borde y la validación, sin necesidad de servicios ni fixtures.

## Risks / Trade-offs

- **GCC ausente** → el `Makefile` falla con un mensaje claro; documentar en
  `AGENTS.md` la instalación de MinGW-w64 y la verificación con `gcc --version`.
- **GNU Make 3.81 antiguo** → limitarse a reglas y variables básicas; evitar
  funciones modernas como `$(file ...)`.
- **Redondeo de punto flotante** → mostrar dos decimales y comparar con
  tolerancia en pruebas; no comparar `double` con `==`.
- **Corrupción del archivo de datos** → cabecera de versión, guardado atómico y
  error recuperable que no borra el archivo existente.
- **Nombres duplicados** → comparación sin distinguir mayúsculas y rechazo al
  crear o renombrar.
- **El archivo `vehicles.dat` depende del directorio de ejecución** → se
  documenta; ejecutar siempre desde la raíz del proyecto o junto al ejecutable.

## Migration Plan

Proyecto nuevo: no hay datos ni código que migrar. Primer arranque crea el
catálogo vacío si `vehicles.dat` no existe. Revertir consiste en eliminar el
ejecutable, el archivo de datos y los artefactos de build; no hay estado externo.

## Open Questions

Ninguna pendiente que afecte specs, enfoque o desglose de tareas.
