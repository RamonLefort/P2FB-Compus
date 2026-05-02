# Proyecto de Desarrollo de Tipos Abstractos de Datos (TADs)

Este repositorio contiene la implementación de los módulos necesarios para el control de periféricos y la lógica de aplicación en sistemas embebidos. El objetivo es garantizar el encapsulamiento y la modularidad mediante la definición de interfaces claras e implementaciones robustas.

## 📋 Lista de Tareas (Progreso del Desarrollo)

A continuación se detalla el listado de TADs. Marca las tareas con una `x` (ejemplo: `[x]`) a medida que completes la implementación y las pruebas unitarias.

### Capa de Abstracción de Hardware (HAL)
- [x] **TAD SIOInt**: Gestión de interrupciones de Entrada/Salida serie (UART).
- [x] **TAD SIOTime**: Control de tiempos y timeouts para comunicación serie.
- [x] **TAD HB**: Implementación del *Heartbeat* para monitorización del estado del sistema.
- [x] **TAD EEPROM**: Gestión de lectura/escritura en memoria no volátil.
- [x] **TAD TIMER**: Abstracción de temporizadores de hardware para la base de tiempos.
- [x] **TAD ADC**: Controlador para la conversión Analógico-Digital.

### Capa de Dispositivos (Drivers)
- [x] **TAD LCD**: Controlador para la visualización de datos en pantalla de cristal líquido.
- [x] **TAD LIGHT**: Gestión del sensor de luminosidad (LDR).
- [x] **TAD JSK**: Interfaz para el manejo de Joystick (ejes X/Y y pulsador).
- [x] **TAD Button**: Lógica de *debouncing* (antirrebotes) y gestión de eventos de pulsadores.

### Capa de Aplicación
- [ ] **TAD ORGANIZER**: Planificador de tareas (Scheduler) para la ejecución cooperativa.
- [x] **TAD ANIMALS**: Lógica de dominio específica para la gestión de entidades "Animals".
