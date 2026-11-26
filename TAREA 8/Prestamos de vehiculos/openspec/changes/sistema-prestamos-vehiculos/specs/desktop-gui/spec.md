# Spec Delta

## Purpose

Provee una interfaz gráfica de escritorio intuitiva para operar todas las funciones del sistema de préstamos vehiculares sin depender de comandos de consola.

## ADDED Requirements

### Requirement: Navegación gráfica por módulos
El sistema SHALL presentar una ventana principal con secciones claramente organizadas para gestionar Clientes, Vehículos, Préstamos y Pagos, y Reportes de Estado de Pago.

#### Scenario: Cambio entre secciones de la interfaz
- **WHEN** el usuario selecciona una pestaña o sección (Clientes, Vehículos, Préstamos/Pagos o Reportes)
- **THEN** la interfaz muestra los controles, formularios y tablas correspondientes a esa sección manteniendo los datos sincronizados

### Requirement: Retroalimentación y validación en interfaz gráfica
El sistema SHALL mostrar mensajes claros de confirmación o error ante operaciones válidas o inválidas, así como la visualización directa del saldo y de la lista de meses pagados/no pagados.

#### Scenario: Notificación de error de validación en GUI
- **WHEN** el usuario envía un formulario con campos faltantes o valores numéricos inválidos
- **THEN** la interfaz muestra un mensaje descriptivo del error y mantiene los datos ingresados para su corrección
