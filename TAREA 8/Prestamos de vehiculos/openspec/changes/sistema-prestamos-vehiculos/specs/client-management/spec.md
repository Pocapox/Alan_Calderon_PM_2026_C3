# Spec Delta

## Purpose

Proporciona la administración de clientes solicitantes de préstamos vehiculares, permitiendo agregar, buscar y borrar registros de clientes de manera consistente.

## ADDED Requirements

### Requirement: Agregar cliente
El sistema SHALL permitir registrar un nuevo cliente con al menos su identificador único, nombre completo, documento de identidad y teléfono/contacto, validando que no existan duplicados por documento de identidad.

#### Scenario: Alta exitosa de cliente
- **WHEN** el usuario ingresa los datos obligatorios de un cliente cuyo documento no existe en el sistema
- **THEN** el sistema agrega el cliente a la lista de clientes y lo hace visible para búsquedas y asignación de préstamos

#### Scenario: Rechazo de cliente duplicado
- **WHEN** el usuario intenta agregar un cliente con un documento de identidad ya registrado
- **THEN** el sistema rechaza el alta y notifica que el documento ya existe

### Requirement: Buscar cliente
El sistema SHALL permitir buscar clientes por identificador, documento de identidad o coincidencia parcial en el nombre, mostrando los resultados coincidentes.

#### Scenario: Búsqueda con coincidencias
- **WHEN** el usuario ingresa un criterio de búsqueda que coincide con uno o más clientes
- **THEN** el sistema muestra únicamente los clientes que coinciden con el criterio ingresado

#### Scenario: Búsqueda sin coincidencias
- **WHEN** el usuario ingresa un criterio que no coincide con ningún cliente registrado
- **THEN** el sistema indica que no se encontraron clientes para ese criterio

### Requirement: Borrar cliente
El sistema SHALL permitir eliminar un cliente existente, impidiendo el borrado si tiene préstamos activos asociados, o confirmando su eliminación cuando no existen dependencias activas.

#### Scenario: Borrado exitoso de cliente sin préstamos
- **WHEN** el usuario selecciona un cliente sin préstamos asociados y confirma su eliminación
- **THEN** el sistema elimina al cliente de la lista y actualiza la vista

#### Scenario: Intento de borrar cliente con préstamo activo
- **WHEN** el usuario intenta borrar un cliente que tiene un préstamo activo
- **THEN** el sistema bloquea la operación e informa que el cliente posee un préstamo activo
