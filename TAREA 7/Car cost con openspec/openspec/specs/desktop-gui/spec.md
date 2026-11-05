# desktop-gui Specification

## Purpose
Proporciona una interfaz gráfica nativa de Windows para administrar vehículos y calcular viajes, reutilizando el núcleo existente y mostrando los resultados sin usar la consola.

## Requirements

### Requirement: Ventana principal con pestañas
La aplicación DEBE (SHALL) abrir una ventana de Windows con dos pestañas visibles: "Vehiculos" y "Calcular viaje".

#### Scenario: Apertura de la aplicación
- **WHEN** el usuario ejecuta `carcost-gui.exe`
- **THEN** aparece la ventana principal con la pestaña "Vehiculos" activa y la pestaña "Calcular viaje" disponible

#### Scenario: Cambio de pestaña
- **WHEN** el usuario selecciona la pestaña "Calcular viaje"
- **THEN** la ventana muestra los controles de cálculo y oculta los de gestión de vehículos

### Requirement: Listar vehículos en la interfaz
La aplicación DEBE (SHALL) mostrar en una lista los vehículos registrados con su nombre y sus consumos urbano y de carretera.

#### Scenario: Catálogo con vehículos
- **WHEN** la aplicación abre con vehículos en `vehicles.dat`
- **THEN** la lista muestra una fila por vehículo con nombre, consumo urbano y consumo de carretera

#### Scenario: Catálogo vacío
- **WHEN** la aplicación abre sin vehículos registrados
- **THEN** la lista aparece vacía y las acciones de edición y cálculo quedan deshabilitadas

### Requirement: Seleccionar un vehículo
La aplicación DEBE (SHALL) cargar en el formulario todos los parámetros del vehículo seleccionado en la lista.

#### Scenario: Selección de un vehículo
- **WHEN** el usuario selecciona una fila de la lista
- **THEN** el formulario muestra nombre, consumos, seguro, neumáticos, mantenimiento, valores, años, impuestos, limpieza y kilometraje anual de ese vehículo

### Requirement: Crear vehículo desde la interfaz
La aplicación DEBE (SHALL) permitir registrar un vehículo nuevo desde el formulario y guardarlo en el almacén local.

#### Scenario: Alta exitosa
- **WHEN** el usuario pulsa "Nuevo", completa los campos con valores válidos y pulsa "Guardar"
- **THEN** el vehículo aparece en la lista, se guarda en `vehicles.dat` y se muestra una confirmación

#### Scenario: Nombre duplicado
- **WHEN** el usuario intenta guardar un vehículo con un nombre ya registrado, sin distinguir mayúsculas
- **THEN** la aplicación muestra un diálogo de error y no modifica el almacén

### Requirement: Editar vehículo desde la interfaz
La aplicación DEBE (SHALL) permitir modificar los parámetros del vehículo seleccionado y persistir los cambios.

#### Scenario: Edición exitosa
- **WHEN** el usuario cambia el seguro anual de un vehículo seleccionado y pulsa "Guardar"
- **THEN** la lista y `vehicles.dat` reflejan el nuevo valor y los demás campos se conservan

### Requirement: Eliminar vehículo con confirmación
La aplicación DEBE (SHALL) pedir confirmación antes de eliminar el vehículo seleccionado.

#### Scenario: Eliminación confirmada
- **WHEN** el usuario pulsa "Eliminar" y confirma el diálogo
- **THEN** el vehículo desaparece de la lista y de `vehicles.dat`

#### Scenario: Eliminación cancelada
- **WHEN** el usuario pulsa "Eliminar" y cancela el diálogo
- **THEN** el vehículo permanece sin cambios

### Requirement: Validar el formulario con diálogos
La aplicación DEBE (SHALL) rechazar un formulario inválido mostrando un diálogo con el motivo y conservando los datos previamente guardados.

#### Scenario: Valor final mayor que el inicial
- **WHEN** el usuario intenta guardar un valor final superior al valor inicial
- **THEN** aparece un diálogo que indica que la depreciación no puede ser negativa y el almacén no cambia

#### Scenario: Campo numérico inválido
- **WHEN** el usuario escribe texto no numérico en un campo de costo
- **THEN** aparece un diálogo de error y no se guarda el vehículo

### Requirement: Calcular un viaje desde la interfaz
La aplicación DEBE (SHALL) calcular el costo de un viaje a partir del vehículo seleccionado, la distancia, los porcentajes urbano/carretera y el precio del combustible por litro.

#### Scenario: Cálculo con reparto mixto
- **WHEN** el usuario selecciona un vehículo e ingresa 100 km, 60% urbano, 40% carretera y 100 DOP por litro
- **THEN** la pestaña muestra el costo total, el costo por kilómetro y el desglose por rubro en DOP con dos decimales

#### Scenario: Entradas inválidas
- **WHEN** los porcentajes no suman 100 o la distancia es cero
- **THEN** la aplicación muestra un diálogo con el error y no presenta un resultado

### Requirement: Mostrar nombres Unicode
La aplicación DEBE (SHALL) aceptar nombres con acentos y `ñ`, y mostrarlos correctamente después de guardar y volver a abrir.

#### Scenario: Nombre con acentos
- **WHEN** el usuario crea un vehículo llamado "Camión de José"
- **THEN** el nombre se muestra igual en la lista tras guardar, cerrar y reabrir la aplicación

### Requirement: Compartir datos con el CLI
La aplicación DEBE (SHALL) usar el mismo archivo `vehicles.dat` que el CLI, de modo que los cambios de uno sean visibles en el otro.

#### Scenario: Vehículo creado en la GUI visible en el CLI
- **WHEN** el usuario crea un vehículo en la GUI y luego ejecuta el CLI desde el mismo directorio
- **THEN** el CLI lista el vehículo con sus parámetros

### Requirement: Informar errores de carga
La aplicación DEBE (SHALL) informar mediante diálogos los problemas al cargar `vehicles.dat`.

#### Scenario: Cabecera inválida
- **WHEN** el archivo tiene una cabecera distinta de `carcost 1`
- **THEN** la aplicación muestra un diálogo de error y se cierra sin sobrescribir el archivo

#### Scenario: Líneas inválidas
- **WHEN** el archivo contiene líneas que no se pueden interpretar
- **THEN** la aplicación muestra un aviso, carga los vehículos válidos y continúa
