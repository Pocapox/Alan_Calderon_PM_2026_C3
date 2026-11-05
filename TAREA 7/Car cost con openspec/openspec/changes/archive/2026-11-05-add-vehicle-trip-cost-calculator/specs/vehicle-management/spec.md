# Spec Delta

## Purpose

Permite administrar el catálogo local de vehículos y sus parámetros de costo, de modo que el cálculo de viajes use datos completos y persistentes entre ejecuciones.

## ADDED Requirements

### Requirement: Crear vehículo
El sistema DEBE (SHALL) permitir registrar un vehículo con un nombre único y todos sus parámetros de costo: consumo urbano y de carretera en km/L, seguro anual, costo de neumáticos por km, mantenimiento por km, valor inicial, años de uso, valor final, impuestos anuales, costo por lavado, lavados por año y kilometraje promedio anual.

#### Scenario: Registro exitoso
- **WHEN** el usuario elige crear un vehículo e ingresa todos los parámetros con valores válidos
- **THEN** el sistema guarda el vehículo, lo confirma en pantalla y lo deja disponible para calcular viajes

#### Scenario: Nombre duplicado
- **WHEN** el usuario intenta crear un vehículo con un nombre que ya existe, sin distinguir mayúsculas
- **THEN** el sistema rechaza la operación e informa que el nombre ya está en uso

### Requirement: Listar vehículos
El sistema DEBE (SHALL) mostrar los vehículos registrados con un identificador o nombre que permita seleccionarlos para editar, eliminar o calcular.

#### Scenario: Catálogo con vehículos
- **WHEN** el usuario solicita listar los vehículos y existen registros
- **THEN** el sistema muestra cada vehículo registrado y su nombre

#### Scenario: Catálogo vacío
- **WHEN** el usuario solicita listar los vehículos y no hay registros
- **THEN** el sistema informa que no hay vehículos registrados y no falla

### Requirement: Editar vehículo
El sistema DEBE (SHALL) permitir modificar los parámetros de un vehículo existente conservando los valores que el usuario no cambie.

#### Scenario: Edición parcial
- **WHEN** el usuario selecciona un vehículo y modifica solo el seguro anual
- **THEN** el sistema actualiza ese campo y conserva intactos los demás parámetros

#### Scenario: Edición inválida
- **WHEN** el usuario intenta guardar un cambio que deja un parámetro fuera de los valores permitidos
- **THEN** el sistema rechaza el cambio, informa el error y mantiene los valores anteriores

### Requirement: Eliminar vehículo
El sistema DEBE (SHALL) permitir eliminar un vehículo existente solicitando confirmación antes de borrarlo.

#### Scenario: Eliminación confirmada
- **WHEN** el usuario elige eliminar un vehículo y confirma la acción
- **THEN** el sistema borra el vehículo y lo excluye de listados y cálculos posteriores

#### Scenario: Eliminación cancelada
- **WHEN** el usuario elige eliminar un vehículo y cancela la confirmación
- **THEN** el sistema conserva el vehículo sin cambios

### Requirement: Persistencia local
El sistema DEBE (SHALL) guardar los vehículos en un archivo local y volver a cargarlos automáticamente al iniciar.

#### Scenario: Datos disponibles tras reiniciar
- **WHEN** el usuario crea un vehículo, cierra el programa y lo vuelve a abrir
- **THEN** el vehículo aparece en el listado con todos sus parámetros

#### Scenario: Archivo ausente en el primer inicio
- **WHEN** el programa inicia y el archivo de datos no existe
- **THEN** el sistema arranca con un catálogo vacío sin mostrar un error fatal

### Requirement: Validar parámetros del vehículo
El sistema DEBE (SHALL) rechazar parámetros que hagan imposible un cálculo coherente: consumos, kilometraje anual y años de uso mayores que cero, costos no negativos y valor final no mayor que el valor inicial.

#### Scenario: Valor final mayor que el inicial
- **WHEN** el usuario ingresa un valor final superior al valor inicial
- **THEN** el sistema rechaza el registro e indica que la depreciación no puede ser negativa

#### Scenario: Consumo urbano en cero
- **WHEN** el usuario ingresa un consumo urbano igual a cero
- **THEN** el sistema rechaza el registro e indica que el consumo debe ser mayor que cero
