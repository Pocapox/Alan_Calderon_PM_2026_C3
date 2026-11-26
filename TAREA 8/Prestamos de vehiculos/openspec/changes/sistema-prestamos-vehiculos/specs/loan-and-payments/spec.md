# Spec Delta

## Purpose

Gestiona la creación de préstamos para la compra de vehículos con tasa de interés anual configurable y cuota fija mensual, el cronograma mensual (marcar meses pagados y no pagados), el registro de pagos mensuales, los pagos de abono a capital y la visualización del saldo pendiente.

## ADDED Requirements

### Requirement: Crear préstamo con tasa de interés anual configurable y cuota fija mensual
El sistema SHALL permitir crear un préstamo asociando un cliente y un vehículo existentes, indicando monto financiado, plazo en meses y tasa de interés anual configurable, calculando automáticamente la cuota fija mensual y generando la lista de meses del cronograma.

#### Scenario: Creación de préstamo con tasa de interés positiva
- **WHEN** el usuario crea un préstamo con cliente válido, vehículo válido, monto mayor a 0, plazo en meses mayor a 0 y tasa anual mayor a 0
- **THEN** el sistema calcula la cuota fija mensual mediante amortización estándar, genera la lista de meses del 1 al plazo total marcados inicialmente como no pagados y establece el saldo pendiente inicial

#### Scenario: Creación de préstamo con tasa de interés 0%
- **WHEN** el usuario crea un préstamo con tasa anual igual a 0
- **THEN** el sistema calcula la cuota fija mensual dividiendo el monto entre el número de meses y genera la lista de meses como no pagados

### Requirement: Lista de meses con estado pagado y no pagado
El sistema SHALL mantener una lista ordenada de los meses del préstamo, permitiendo visualizar y gestionar explícitamente qué meses están marcados como pagados y cuáles como no pagados.

#### Scenario: Marcar un mes como pagado y revertir a no pagado
- **WHEN** el usuario registra el pago mensual de un mes pendiente o cambia el estado de un mes entre pagado y no pagado
- **THEN** el sistema actualiza el estado del mes en la lista de meses y recalcula el saldo pendiente y los totales pagados

### Requirement: Gestionar pagos mensuales
El sistema SHALL permitir registrar el pago correspondiente a una cuota mensual del préstamo, marcando el mes como pagado y registrando la fecha y el monto pagado.

#### Scenario: Registro exitoso de pago mensual
- **WHEN** el usuario selecciona un mes no pagado y confirma el pago mensual
- **THEN** el sistema marca ese mes como pagado, descuenta el aporte correspondiente del saldo pendiente y actualiza la visualización del saldo

#### Scenario: Intento de pagar un mes ya pagado
- **WHEN** el usuario intenta registrar un pago mensual sobre un mes que ya figura como pagado
- **THEN** el sistema informa que el mes ya fue pagado y evita duplicar el cobro

### Requirement: Agregar pagos de abono
El sistema SHALL permitir registrar pagos de abono (pagos extraordinarios o parciales adicionales a la cuota mensual) que reducen directamente el saldo pendiente del préstamo.

#### Scenario: Abono válido reduce el saldo pendiente
- **WHEN** el usuario ingresa un monto de abono mayor a 0 y menor o igual al saldo pendiente actual
- **THEN** el sistema registra el abono con su monto y fecha, reduce el saldo pendiente en ese monto y actualiza el estado del préstamo (marcándolo como liquidado si el saldo llega a 0)

#### Scenario: Rechazo de abono superior al saldo pendiente
- **WHEN** el usuario intenta registrar un abono con monto menor o igual a 0, o mayor al saldo pendiente actual
- **THEN** el sistema rechaza el abono e informa el límite válido sin alterar el saldo

### Requirement: Visualizar el saldo del préstamo
El sistema SHALL mostrar en todo momento el saldo pendiente actualizado, el total pagado en cuotas mensuales, el total abonado y el progreso de meses pagados frente al total de meses.

#### Scenario: Actualización inmediata del saldo tras pago o abono
- **WHEN** se registra un pago mensual, un cambio de estado de mes o un pago de abono
- **THEN** la vista del préstamo muestra inmediatamente el nuevo saldo pendiente y el resumen actualizado
