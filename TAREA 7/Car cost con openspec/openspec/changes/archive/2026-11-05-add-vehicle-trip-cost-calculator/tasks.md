# Tasks

## 1. Entorno y esqueleto del proyecto

- [x] 1.1 Verificar el toolchain: ejecutar `gcc --version` y `make --version`; si GCC no existe, instalar MinGW-w64 y agregar su carpeta `bin` al PATH. Verificación: ambos comandos responden sin error.
- [x] 1.2 Crear el `Makefile` con objetivos `all`, `test` y `clean`, banderas `-std=c11 -Wall -Wextra -Wpedantic` y salida `carcost.exe`. Verificación: `make -n` muestra los comandos de compilación esperados.
- [x] 1.3 Crear `src/` y `tests/` con los encabezados y archivos vacíos de los módulos `vehicle`, `storage`, `calculator` y `main`, más un `main` mínimo. Verificación: `make` compila y `carcost.exe` inicia y termina sin error.

## 2. Modelo y validación de vehículos

- [x] 2.1 Definir la estructura `Vehicle` con nombre, consumos urbano/carretera, seguro, neumáticos/km, mantenimiento/km, valor inicial, años de uso, valor final, impuestos, costo por lavado, lavados/año y km/año. Verificación: `make` compila sin advertencias.
- [x] 2.2 Implementar la validación de rangos (consumos, km/año y años > 0; costos >= 0; valor final <= inicial) con mensajes de error claros. Verificación: pruebas unitarias de cada regla pasan con `make test`.
- [x] 2.3 Implementar la comparación de nombres sin distinguir mayúsculas y el rechazo de duplicados. Verificación: prueba unitaria que registra `Auto` y rechaza `auto` pasa.

## 3. Motor de cálculo

- [x] 3.1 Implementar depreciación anual, limpieza anual, total anual y costo fijo por km como funciones puras. Verificación: pruebas con los ejemplos de las specs (depreciación 120000, limpieza 6000, total 96000, fijo 1000 DOP) pasan.
- [x] 3.2 Implementar el reparto de distancia y el combustible por tramo sumando litros (no promediando rendimientos). Verificación: la prueba 60 km urbano a 10 km/L más 40 km carretera a 20 km/L da 800 DOP con litro a 100 DOP.
- [x] 3.3 Implementar el costo variable, el costo total del viaje y el costo por km, con tolerancia `1e-9` en las comparaciones. Verificación: una prueba de extremo a extremo del cálculo coincide con el resultado esperado.
- [x] 3.4 Cubrir casos borde: 100% urbano, 100% carretera, distancia cero, km/año cero y porcentajes distintos de 100. Verificación: cada caso tiene una prueba que espera rechazo o resultado correcto.

## 4. Persistencia local

- [x] 4.1 Implementar el guardado y la carga en `vehicles.dat` con cabecera de versión y campos separados por `|`. Verificación: prueba de ida y vuelta que guarda varios vehículos y los recupera idénticos.
- [x] 4.2 Implementar guardado atómico mediante `vehicles.tmp` y renombrado, y el arranque con catálogo vacío si el archivo no existe. Verificación: pruebas confirman que no queda un archivo temporal y que un primer inicio no falla.
- [x] 4.3 Manejar líneas malformadas y nombres con `|` o saltos de línea sin corromper el archivo existente. Verificación: prueba con una línea inválida reporta el error y conserva los vehículos válidos.

## 5. Interfaz de consola y CRUD

- [x] 5.1 Implementar helpers de entrada de texto, números (aceptando `,` y `.` como separador decimal) y confirmaciones sí/no, rechazando texto sobrante. Verificación: pruebas unitarias de entradas válidas e inválidas pasan.
- [x] 5.2 Implementar el menú principal y las acciones crear y listar vehículos con todos sus campos. Verificación: una sesión con entrada simulada crea un vehículo y el listado lo muestra.
- [x] 5.3 Implementar edición parcial y eliminación con confirmación, conservando campos no modificados. Verificación: sesión simulada edita un campo y cancela/confirma una eliminación con los resultados esperados.
- [x] 5.4 Probar el CRUD completo desde la consola con datos reales y verificar que los cambios sobreviven al reinicio. Verificación: cerrar y abrir `carcost.exe` muestra el estado final esperado.

## 6. Cálculo desde la interfaz

- [x] 6.1 Implementar la selección de vehículo y la captura de distancia, porcentajes y precio del combustible. Verificación: entrada simulada con 60/40 y precio 100 produce el cálculo sin errores.
- [x] 6.2 Mostrar costo total, costo por km y desglose por rubro en DOP con dos decimales y mensajes de validación para entradas incorrectas. Verificación: la salida simulada coincide con los valores esperados y los casos inválidos muestran el error correspondiente.

## 7. Documentación y verificación final

- [x] 7.1 Actualizar `AGENTS.md`: reemplazar el estado vacío por la estructura real, los comandos `make`, `make test` y la ejecución de `carcost.exe`, y el requisito de MinGW-w64. Verificación: cada comando documentado se ejecuta tal cual.
- [x] 7.2 Ejecutar la verificación completa: `make clean`, `make` y `make test`. Verificación: los tres comandos terminan con código 0 y sin advertencias.
- [x] 7.3 Hacer una prueba de extremo a extremo desde un `vehicles.dat` inexistente: crear, calcular, editar, reiniciar, calcular de nuevo y eliminar. Verificación: el resultado coincide con el cálculo manual del mismo escenario.

## Workflow follow-up

- Validar los artefactos con `openspec validate add-vehicle-trip-cost-calculator --strict`.
- Archivar el cambio cuando la implementación y las pruebas estén completas.
