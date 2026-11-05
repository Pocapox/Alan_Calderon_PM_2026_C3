# Proposal

## Why

No existe una herramienta que permita saber cuánto cuesta realmente usar un
vehículo por kilómetro ni cuánto cuesta un viaje concreto. Los costos de
propiedad (seguro, impuestos, depreciación, limpieza) se pagan por año y suelen
ignorarse al estimar un viaje, mientras que el combustible varía según el tramo
sea urbano o de carretera. Este proyecto crea una calculadora en C que combina
ambos tipos de costo y permite administrar varios vehículos.

## What Changes

- Nuevo programa de consola en C con menú interactivo.
- CRUD de vehículos: crear, listar, editar y eliminar.
- Cada vehículo almacena: consumo urbano y de carretera (km/L), seguro anual,
  costo de neumáticos por km, mantenimiento por km, valor inicial, años de uso,
  valor final, impuestos anuales, costo por lavado, lavados por año y kilometraje
  promedio anual.
- Cálculo de un viaje a partir de distancia total y porcentaje urbano/carretera.
- Salida: costo total del viaje y costo por kilómetro, con desglose por rubro.
- Persistencia local de los vehículos entre ejecuciones.
- Unidades fijas: peso dominicano (DOP), precio de combustible por litro y
  consumo en km/L.
- Validación de entradas y manejo de casos límite (distancia cero, km/año cero,
  porcentajes que no suman 100, depreciación negativa).

## Capabilities

### New Capabilities

- `vehicle-management`: alta, consulta, edición, eliminación y persistencia de
  vehículos con todos sus parámetros de costo.
- `trip-cost-calculation`: cálculo determinista del costo de un viaje y del
  costo por kilómetro a partir de la distancia y del reparto urbano/carretera.

### Modified Capabilities

- Ninguna: el proyecto no tiene specs existentes.

## Impact

- Proyecto C nuevo, sin código previo. Requiere instalar MinGW-w64 (GCC) y
  añadirlo al PATH; hoy no está disponible.
- No se usan dependencias externas: persistencia en archivo de texto local.
- `AGENTS.md` y la configuración de OpenSpec ya existen; este cambio solo añade
  artefactos de planificación bajo `openspec/changes/add-vehicle-trip-cost-calculator/`.
- El cálculo asume depreciación lineal: `(valor_inicial - valor_final) / años_uso`.
- El costo de limpieza anual se deriva de `costo_por_lavado * lavados_por_ano`.
