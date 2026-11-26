# Spec Delta

## Purpose

Genera reportes detallados del estado de pago de cada préstamo, consolidando información del cliente, vehículo, lista de meses pagados y no pagados, historial de abonos y saldo pendiente.

## ADDED Requirements

### Requirement: Generar reporte de estado de pago
El sistema SHALL generar un reporte estructurado del estado de pago de un préstamo seleccionado, incluyendo datos del cliente, vehículo, condiciones del préstamo, desglose de meses pagados y no pagados, historial de abonos y saldo pendiente.

#### Scenario: Visualización de reporte completo en pantalla
- **WHEN** el usuario solicita el reporte de estado de pago para un préstamo existente
- **THEN** el sistema genera y muestra el reporte completo con la lista de meses (marcando pagados y no pagados), el detalle de abonos y el saldo pendiente actual

#### Scenario: Exportación del reporte de estado de pago
- **WHEN** el usuario solicita exportar el reporte de estado de pago a un archivo de texto legible
- **THEN** el sistema guarda el reporte formateado en disco e informa la ruta del archivo generado
