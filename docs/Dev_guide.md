# Guia de Desarrollo

## Objetivo

Desarrollar una TUI en C para crear y editar texto sencillo, comprimirlo linea por
linea con Zstd, descomprimirlo sin perdida y mostrar estadisticas reales parciales
del proceso.

La estructura inicial no implementa funcionalidades. Cada modulo expone interfaces
para que tres personas puedan trabajar en paralelo con baja dependencia entre si.

## Reglas de trabajo

- Mantener las interfaces publicas en `include/`.
- Evitar mezclar responsabilidades entre modulos.
- No instalar dependencias desde el proyecto.
- Compilar con `make build` antes de integrar cambios.
- Revisar memoria con `make valgrind` cuando haya manejo dinamico de memoria.
- Documentar decisiones tecnicas relevantes en este archivo o en `README.md`.

## Persona 1: TUI y editor

Responsabilidad principal: experiencia de usuario en terminal y edicion de texto.

Tareas:

- Implementar pantallas con `ncurses` en `src/tui.c`.
- Crear menu principal para nuevo archivo, abrir, guardar, comprimir, descomprimir,
  ver estadisticas y salir.
- Implementar editor de texto sencillo en `src/editor.c`.
- Definir limites claros de buffer, cursor, insercion, borrado y navegacion.
- Mostrar mensajes de error generados por otros modulos sin resolverlos dentro de
  la TUI.
- Preparar una pantalla de estadisticas que consuma datos desde `stats.h`.

Criterios de aceptacion:

- La TUI compila sin depender de la implementacion interna de compresion.
- El editor puede probarse con texto en memoria.
- La pantalla de estadisticas se alimenta desde una estructura `StatsReport`.

## Persona 2: Compresion, descompresion y estrategia de escritura

Responsabilidad principal: algoritmo Zstd y persistencia comprimida eficiente.

Tareas:

- Implementar compresion sin perdida usando `libzstd-dev` en `src/compression.c`.
- Procesar el contenido por lineas cuando el flujo de datos lo permita.
- Implementar descompresion y validar que el resultado conserve el contenido.
- Preparar escritura por bloques de 4KB para reducir llamadas pequenas a `write()`.
- Evaluar si `mmap` aporta valor para lectura o escritura y documentar la decision.
- Reportar datos de volumen comprimido y cantidad de escrituras al modulo de
  estadisticas.

Criterios de aceptacion:

- Un archivo descomprimido debe ser igual al original.
- Los errores de Zstd se devuelven con codigos claros.
- El modulo no depende de `ncurses`.

## Persona 3: Archivos, estadisticas, pruebas y memoria

Responsabilidad principal: entrada/salida, medicion y calidad.

Tareas:

- Implementar lectura y escritura de archivos en `src/file_io.c`.
- Integrar mediciones en `src/stats.c`.
- Medir volumen de datos escrito a disco.
- Contar llamadas de escritura controladas por el programa.
- Medir tiempo de CPU en user mode, tiempo de sistema y tiempo total wall-clock.
- Crear archivos de ejemplo en `data/`.
- Agregar pruebas en `tests/` para casos basicos y errores.
- Mantener el objetivo `make valgrind` util para detectar fugas.

Criterios de aceptacion:

- Las estadisticas se pueden obtener sin depender de la TUI.
- Las pruebas cubren lectura, escritura y comparacion de archivos.
- Valgrind no reporta fugas en flujos principales implementados.

## Estadisticas a mostrar

La vista de estadisticas debe comparar el enfoque clasico con el enfoque propuesto.
Los valores deben medirse parcialmente cuando sea posible, no quedar como una tabla
fija.

Metricas objetivo:

- Volumen de datos a disco.
- Llamadas a `write()`.
- Tiempo de CPU en user mode.
- Tiempo de SO en sys mode.
- Tiempo total wall-clock.

Referencia teorica para el informe:

| Metrica del Kernel | Enfoque Clasico | Enfoque Propuesto | Impacto |
| --- | --- | --- | --- |
| Volumen de Datos a Disco | 50 MB | 15 MB | -70% |
| Llamadas a write() | 12,800 | 3,750 | -70% |
| Tiempo de CPU (User Mode) | 0.01 ms | 35.0 ms | Aumento esperado por compresion |
| Tiempo de SO (Sys Mode) | 15.0 ms | 4.0 ms | -73% |
| Tiempo Total (Wall-clock) | 120.5 ms | 85.0 ms | Sistema 29% mas rapido |

## Orden recomendado

1. Acordar contratos de datos en headers.
2. Implementar cada modulo con pruebas locales.
3. Integrar TUI con editor y archivos.
4. Integrar compresion/descompresion.
5. Integrar estadisticas.
6. Ejecutar pruebas y Valgrind.

## Decisiones implementadas para Personas 2 y 3

- `src/compression.c` mantiene la compresion por registros de linea con frames Zstd
  independientes y escritura acumulada en bloques de 4 KB.
- La escritura basada en `write()` maneja escrituras parciales y cuenta cada llamada
  controlada por el programa.
- `src/file_io.c` implementa lectura completa y escritura completa sin depender de
  `ncurses`; las variantes `*_with_stats` acumulan metricas cuando reciben un
  `StatsReport`.
- `src/stats.c` centraliza inicializacion, acumulacion de volumenes, llamadas a
  escritura y medicion de CPU user, CPU sys y wall-clock.
- `make test` ejecuta pruebas de compresion/descompresion, file I/O y estadisticas.
- `make valgrind` ejecuta Valgrind sobre los flujos no-TUI cubiertos por pruebas.
