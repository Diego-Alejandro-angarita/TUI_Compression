# Tareas Del Equipo

## Integrante 1: Pipeline De Compresion Y Buffers

Objetivo: implementar el manejo robusto de buffers y la compresion/descompresion ZSTD sobre memoria.

Archivos asignados: `include/buffer.h`, `src/buffer.c`, `include/compression.h`, `src/compression.c`, `tests/test_buffer.c`, `tests/test_compression_stub.c`.

Funciones principales futuras: `buffer_reserve`, `buffer_clear`, `compress_buffer`, `decompress_buffer`.

Criterios de finalizacion: buffers sin fugas, compresion round-trip con ZSTD, manejo correcto de archivos vacios representados como buffers, pruebas automatizadas verdes.

Dependencias: recibe configuracion de algoritmo desde `io_pipeline`; entrega tamanos comprimidos para `file_format` y profiling.

No debe tocar: `crypto.c`, `key_manager.c`, `file_format.c`, `io_pipeline.c` salvo cambios coordinados de interfaz.

## Integrante 2: Seguridad Criptografica Y Llaves

Objetivo: implementar cifrado simetrico por buffer y manejo seguro de llaves temporales.

Archivos asignados: `include/crypto.h`, `src/crypto.c`, `include/key_manager.h`, `src/key_manager.c`, `include/secure_memory.h`, `src/secure_memory.c`, `tests/test_crypto_stub.c`.

Funciones principales futuras: `encrypt_buffer`, `decrypt_buffer`, `key_manager_request_password_stub`, `key_manager_derive_key_stub`, `key_manager_destroy_key`, `secure_zero`.

Criterios de finalizacion: cifrado/descifrado round-trip, nonce obligatorio, KDF real definida, llaves limpiadas de memoria, pruebas de limpieza segura y errores de entrada.

Dependencias: requiere `salt` y `nonce` desde `file_format`; consume buffers comprimidos generados por Integrante 1.

No debe tocar: `compression.c`, `buffer.c`, `profiler.c`, `tui.c` salvo ajustes acordados de estructuras compartidas.

## Integrante 3: Formato De Archivo, Profiling E Integracion General

Objetivo: coordinar el flujo guardar/cargar, definir el header final y medir tiempos por etapa.

Archivos asignados: `include/file_format.h`, `src/file_format.c`, `include/io_pipeline.h`, `src/io_pipeline.c`, `include/profiler.h`, `src/profiler.c`, `tests/test_pipeline.c`, `docs/`.

Funciones principales futuras: `file_format_create_header`, `file_format_validate_header`, `io_pipeline_save_stub`, `io_pipeline_load_stub`, `profiler_timer_start`, `profiler_timer_stop`.

Criterios de finalizacion: header validado, flujo completo conectado, profiling por etapa, pruebas del pipeline general, documentacion actualizada.

Dependencias: necesita compresion funcional del Integrante 1 y cifrado/llaves del Integrante 2.

No debe tocar: implementaciones internas de ZSTD o cifrado; solo debe usar sus interfaces publicas.
