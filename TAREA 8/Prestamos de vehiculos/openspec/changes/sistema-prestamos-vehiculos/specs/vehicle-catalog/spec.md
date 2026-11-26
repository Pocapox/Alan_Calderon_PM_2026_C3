# Spec Delta

## Purpose

Permite administrar el catálogo de vehículos disponibles para financiamiento, registrando marca, modelo y año, y consultándolos o eliminándolos según las operaciones del sistema.

## ADDED Requirements

### Requirement: Registrar vehículo con marca, modelo y año
El sistema SHALL permitir registrar un nuevo vehículo especificando obligatoriamente su marca, modelo y año válido, asignándole un identificador único.

#### Scenario: Registro exitoso de vehículo
- **WHEN** el usuario ingresa una marca no vacía, un modelo no vacío y un año numérico válido (por ejemplo, entre 1950 y el año siguiente al actual) y confirma el registro
- **THEN** el sistema crea el vehículo, lo agrega al catálogo en memoria y lo muestra en la lista de vehículos disponibles

#### Scenario: Rechazo de vehículo con datos inválidos
- **WHEN** el usuario intenta registrar un vehículo con marca vacía, modelo vacío o un año fuera de rango
- **THEN** el sistema rechaza el registro e informa al usuario qué campo es inválido sin modificar el catálogo

### Requirement: Listar y eliminar vehículos
El sistema SHALL permitir visualizar todos los vehículos registrados y eliminar un vehículo que no esté asociado a un préstamo activo.

#### Scenario: Eliminación de vehículo sin préstamo activo
- **WHEN** el usuario selecciona un vehículo sin préstamos activos asociados y solicita eliminarlo
- **THEN** el sistema remueve el vehículo del catálogo y libera los recursos asociados

#### Scenario: Protección de vehículo con préstamo activo
- **WHEN** el usuario intenta eliminar un vehículo vinculado a un préstamo existente
- **THEN** el sistema impide la eliminación e informa que el vehículo está asociado a un préstamo
