# Spec Delta

## Purpose

Asegura la persistencia de toda la información del sistema en archivos JSON locales, permitiendo guardar y restaurar clientes, vehículos, préstamos, lista de meses y pagos de abono entre ejecuciones.

## ADDED Requirements

### Requirement: Guardar estado completo en archivo JSON
El sistema SHALL serializar y guardar en un archivo JSON todos los clientes, vehículos, préstamos, sus listas de meses con estado pagado/no pagado y sus registros de abonos.

#### Scenario: Guardado automático o manual exitoso
- **WHEN** el usuario guarda cambios o cierra la aplicación tras modificar clientes, vehículos o préstamos
- **THEN** el sistema escribe un archivo JSON válido con la estructura completa de datos en la ruta configurada

### Requirement: Cargar estado completo desde archivo JSON
El sistema SHALL leer y deserializar el archivo JSON al iniciar la aplicación para reconstruir en memoria las listas de clientes, vehículos, préstamos, meses y abonos.

#### Scenario: Inicio con archivo JSON existente
- **WHEN** la aplicación inicia y existe un archivo JSON de datos válido
- **THEN** el sistema carga todos los registros reconstruyendo las relaciones entre clientes, vehículos y préstamos

#### Scenario: Inicio sin archivo JSON previo
- **WHEN** la aplicación inicia y no existe archivo JSON previo
- **THEN** el sistema inicializa las estructuras vacías sin errores y crea el archivo al primer guardado
