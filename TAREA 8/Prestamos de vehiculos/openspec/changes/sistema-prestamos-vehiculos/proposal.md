# Proposal

## Why

Se requiere un sistema de escritorio en lenguaje C para administrar préstamos destinados a la compra de vehículos, permitiendo gestionar clientes, catálogo de vehículos, cronogramas de pagos mensuales y abonos a capital de forma estructurada. Al ser un proyecto nuevo, se necesita establecer la arquitectura con listas enlazadas, memoria dinámica y persistencia en archivos JSON antes de iniciar la implementación.

## What Changes

- Se crea una aplicación de escritorio (GUI) en lenguaje C para Windows (Win32 API) sin dependencias externas complejas.
- Se implementan estructuras de datos basadas en listas enlazadas simples/dobles con asignación y liberación explícita de memoria dinámica (`malloc`/`realloc`/`free`).
- Se incorpora gestión de vehículos (marca, modelo y año) asociada a los préstamos.
- Se incorpora gestión de clientes con operaciones para agregar, buscar y borrar registros.
- Se implementa el cálculo de préstamos vehiculares con tasa de interés anual configurable (cuota fija mensual), generación de lista de meses, marcado de meses pagados y no pagados, registro de pagos mensuales y pagos de abono que reducen el saldo pendiente.
- Se agrega visualización en tiempo real del saldo pendiente y generación de reportes de estado de pago.
- Se implementa persistencia completa de datos en archivos JSON.

## Capabilities

### New Capabilities
- `vehicle-catalog`: Gestión de vehículos (marca, modelo y año) en memoria dinámica mediante listas enlazadas.
- `client-management`: Gestión de clientes (agregar, buscar y eliminar) en memoria dinámica mediante listas enlazadas.
- `loan-and-payments`: Creación de préstamos con tasa de interés anual configurable y cuota fija mensual, lista de meses con estado pagado/no pagado, gestión de pagos mensuales, pagos de abono que reducen el saldo pendiente y visualización del saldo.
- `payment-reporting`: Generación y visualización de reportes de estado de pago del préstamo (detalle de cuotas, meses pagados/pendientes, abonos aplicados y saldo actual).
- `json-persistence`: Carga y guardado del estado completo del sistema (clientes, vehículos, préstamos, meses y abonos) en archivos JSON.
- `desktop-gui`: Interfaz gráfica de escritorio en C (Win32 API) que integra la gestión de clientes, vehículos, préstamos, pagos, abonos, visualización de saldo y reportes.

### Modified Capabilities
<!-- Proyecto nuevo: no existen capacidades previas que modificar. -->

## Impact

- **Código nuevo**: Se creará la estructura fuente en C (`src/`, `include/`), módulos de listas enlazadas, lógica financiera, persistencia JSON (`cJSON` o módulo JSON integrado), interfaz gráfica Win32 y archivo de compilación (`CMakeLists.txt` / `Makefile` para `gcc` MinGW-w64).
- **Datos**: Se generará y leerá un archivo JSON local (por ejemplo `data/prestamos.json`) para persistir los registros entre sesiones.
- **Plataforma objetivo**: Windows x64 compilado con `gcc` (MinGW-w64 / WinLibs).
