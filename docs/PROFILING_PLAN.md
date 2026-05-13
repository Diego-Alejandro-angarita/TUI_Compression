# Plan De Profiling

El modulo `profiler` prepara una API para medir tiempos separados sin acoplarse a ZSTD, OpenSSL ni escritura real.

## Metricas

- Tiempo de compresion: desde que entra el texto plano hasta que se obtiene el buffer comprimido.
- Tiempo de cifrado: desde que entra el buffer comprimido hasta que se obtiene el buffer cifrado.
- Tiempo de escritura: desde que se prepara el header hasta que se termina de escribir el payload.
- Tiempo total: flujo completo de guardado o carga.
- Comparacion contra guardado clasico: escritura de texto plano sin compresion ni cifrado.

## Implementacion Futura

- Usar reloj monotono para tiempo real.
- Mantener contadores por etapa dentro de `ProfileReport`.
- Registrar bytes originales, comprimidos, cifrados y escritos.
- Reportar numero de llamadas futuras a `write()` cuando se implemente I/O real.

## Uso Esperado

`io_pipeline` iniciara y detendra timers alrededor de cada etapa. Los modulos internos no deben imprimir resultados ni conocer la UI.
