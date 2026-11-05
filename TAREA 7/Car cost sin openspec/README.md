# Calculadora de Costo de Viaje por Kilómetro en C (Interfaz Gráfica)

Aplicación nativa de escritorio desarrollada en **lenguaje C puro** con la API gráfica oficial de Windows (**Win32 API** y **Common Controls**), que calcula el costo total y por kilómetro de un viaje vehicular considerando tanto costos directos como fijos y prorrateados.

---

## 🚗 Parámetros del Vehículo Soportados (CRUD Completo)

La aplicación permite **Crear**, **Editar**, **Eliminar** y **Guardar** vehículos con los siguientes 7 parámetros requeridos:

1. **Consumo de Combustible:**
   - En Ciudad (L / 100 km).
   - En Carretera (L / 100 km).
   - Precio de referencia del combustible ($/Litro).
2. **Costo de Seguro:**
   - Póliza anual ($/año).
3. **Costo de Neumáticos:**
   - Desgaste por kilómetro ($/km).
4. **Costo de Mantenimiento:**
   - Servicio mecánico y fluidos por kilómetro ($/km).
5. **Depreciación del Vehículo:**
   - Valor inicial de compra ($).
   - Tiempo de uso proyectado (años).
   - Valor final o residual de reventa ($).
   - Kilómetros anuales estimados (km/año) para prorratear con exactitud el desgaste por km.
6. **Costos de Impuestos:**
   - Patente, matriculación y tasas por año ($/año).
7. **Costo de Limpieza:**
   - Lavados y mantenimiento estético por año ($/año).

---

## 🛣️ Entradas del Viaje

Para cada cálculo de viaje, el usuario puede ingresar:
- **Distancia total del viaje (km).**
- **Porcentaje en Carretera (%):** Con slider interactivo y caja numérica.
- **Porcentaje en Ciudad (%):** Se sincroniza automáticamente (`100% - Carretera`).
- **Precio del combustible para el viaje ($/L):** Precargado desde el vehículo y modificable según la estación de servicio.

---

## 📊 Resultados y Desglose

1. **Costo Total del Viaje ($)** en tarjeta destacada.
2. **Costo por Kilómetro ($/km)** en tarjeta destacada.
3. **Tabla Desglosada (ListView):**
   - Combustible ($ total, $/km, % del viaje y litros consumidos).
   - Neumáticos ($ total, $/km, %).
   - Mantenimiento ($ total, $/km, %).
   - Depreciación ($ total, $/km, %).
   - Seguro anual prorrateado ($ total, $/km, %).
   - Impuestos anuales prorrateados ($ total, $/km, %).
   - Limpieza anual prorrateada ($ total, $/km, %).
   - **TOTAL GENERAL** consolidado.
4. **Datos complementarios:**
   - Consumo medio mixto ponderado (L/100 km).
   - Litros de combustible consumidos en el viaje.
5. **Botón "Copiar Reporte":** Exporta un resumen formateado directamente al portapapeles de Windows.

---

## 💾 Persistencia de Datos

Todos los vehículos se guardan automáticamente en formato legible en `vehiculos.csv` en el mismo directorio, por lo que las modificaciones persisten entre sesiones.

---

## 🛠️ Cómo Compilar y Ejecutar

### Opción 1: Con el script automático (Recomendado)
Haz doble clic en:
```cmd
build.bat
```

### Opción 2: Compilación manual con GCC (MinGW / Clang)
```powershell
gcc -O2 -Wall -mwindows main.c vehicle.c dlg_vehicle.c -o CalculadoraCostoViaje.exe -lcomctl32 -lgdi32 -luser32 -lkernel32
```

### Opción 3: Con Make
```powershell
make
```

### Ejecutar directamente:
```powershell
.\CalculadoraCostoViaje.exe
```
