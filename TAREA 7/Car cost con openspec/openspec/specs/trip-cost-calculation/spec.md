# trip-cost-calculation Specification

## Purpose
Calcula de forma determinista el costo total de un viaje y su costo por kilómetro, combinando combustible por tramo con costos variables y costos anuales prorrateados del vehículo.

## Requirements

### Requirement: Entradas del cálculo
El sistema DEBE (SHALL) solicitar el vehículo, la distancia total en kilómetros, el porcentaje de carretera, el porcentaje urbano y el precio del combustible por litro en DOP.

#### Scenario: Cálculo con reparto mixto
- **WHEN** el usuario elige un vehículo e ingresa distancia, porcentajes y precio del combustible válidos
- **THEN** el sistema realiza el cálculo usando esos valores y muestra el resultado

### Requirement: Distribuir la distancia por tipo de vía
El sistema DEBE (SHALL) dividir la distancia total en tramo urbano y tramo de carretera según los porcentajes ingresados.

#### Scenario: Reparto 60/40
- **WHEN** el usuario ingresa 100 km con 60% urbano y 40% de carretera
- **THEN** el sistema usa 60 km urbanos y 40 km de carretera en el cálculo

#### Scenario: Viaje totalmente urbano
- **WHEN** el usuario ingresa 100% urbano y 0% de carretera
- **THEN** el sistema calcula todo el combustible con el consumo urbano

### Requirement: Calcular combustible por tramo
El sistema DEBE (SHALL) calcular el combustible de cada tramo dividiendo sus kilómetros entre el consumo correspondiente en km/L y multiplicando por el precio por litro; la suma de ambos tramos es el costo de combustible del viaje.

#### Scenario: Rendimientos distintos
- **WHEN** el vehículo tiene consumo urbano de 10 km/L y de carretera de 20 km/L, el viaje es 60 km urbano y 40 km de carretera, y el litro cuesta 100 DOP
- **THEN** el costo de combustible es 600 DOP en ciudad más 200 DOP en carretera, para 800 DOP en total

### Requirement: Calcular costos variables por kilómetro
El sistema DEBE (SHALL) sumar el costo de neumáticos por km y el de mantenimiento por km, y multiplicar esa suma por la distancia total.

#### Scenario: Costos variables del viaje
- **WHEN** el vehículo tiene 2 DOP/km de neumáticos y 3 DOP/km de mantenimiento y el viaje es de 100 km
- **THEN** el costo variable del viaje es 500 DOP

### Requirement: Prorratear costos anuales por kilometraje
El sistema DEBE (SHALL) calcular un costo fijo por kilómetro dividiendo el total anual entre el kilometraje promedio anual del vehículo, y multiplicarlo por la distancia del viaje.

#### Scenario: Prorrateo de costos anuales
- **WHEN** el total anual del vehículo es 120000 DOP, su kilometraje promedio anual es 12000 km y el viaje es de 100 km
- **THEN** el costo fijo imputado al viaje es 1000 DOP

### Requirement: Calcular el total anual del vehículo
El sistema DEBE (SHALL) obtener el total anual sumando el seguro anual, los impuestos anuales, la depreciación anual y la limpieza anual.

#### Scenario: Composición del total anual
- **WHEN** el vehículo tiene 30000 DOP de seguro, 10000 DOP de impuestos, 50000 DOP de depreciación anual y 6000 DOP de limpieza anual
- **THEN** el total anual es 96000 DOP

### Requirement: Calcular depreciación anual
El sistema DEBE (SHALL) calcular la depreciación anual como la diferencia entre el valor inicial y el valor final dividida entre los años de uso.

#### Scenario: Depreciación lineal
- **WHEN** el valor inicial es 1000000 DOP, el valor final es 400000 DOP y los años de uso son 5
- **THEN** la depreciación anual es 120000 DOP

### Requirement: Calcular limpieza anual
El sistema DEBE (SHALL) calcular la limpieza anual multiplicando el costo por lavado por la cantidad de lavados por año.

#### Scenario: Limpieza por frecuencia
- **WHEN** cada lavado cuesta 500 DOP y el vehículo se lava 12 veces al año
- **THEN** la limpieza anual es 6000 DOP

### Requirement: Reportar costo del viaje y costo por kilómetro
El sistema DEBE (SHALL) mostrar el costo total del viaje y el costo por kilómetro, junto con el desglose de combustible, costos variables, costos anuales prorrateados, depreciación y limpieza.

#### Scenario: Resultado completo
- **WHEN** el cálculo termina con entradas válidas y distancia mayor que cero
- **THEN** el sistema muestra el costo total del viaje, el costo por kilómetro y el desglose por rubro en DOP con dos decimales

### Requirement: Validar entradas del cálculo
El sistema DEBE (SHALL) rechazar un cálculo cuando la distancia o el precio del combustible no sean mayores que cero, cuando la suma de porcentajes no sea 100 o cuando el kilometraje anual del vehículo sea cero.

#### Scenario: Porcentajes incorrectos
- **WHEN** el usuario ingresa 50% urbano y 40% de carretera
- **THEN** el sistema rechaza el cálculo e indica que los porcentajes deben sumar 100

#### Scenario: Distancia cero
- **WHEN** el usuario ingresa una distancia total de 0 km
- **THEN** el sistema rechaza el cálculo e indica que la distancia debe ser mayor que cero
