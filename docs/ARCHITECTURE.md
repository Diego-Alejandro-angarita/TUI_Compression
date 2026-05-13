# Arquitectura

El proyecto queda dividido en modulos pequenos para que tres integrantes puedan trabajar en paralelo sin mezclar responsabilidades.

## Modulos

- `buffer`: define `Buffer`, reserva memoria, limpia contenido y libera recursos.
- `compression`: expone `compress_buffer` y `decompress_buffer`; mas adelante integrara ZSTD sobre buffers en RAM.
- `crypto`: expone `encrypt_buffer` y `decrypt_buffer`; mas adelante cifrara buffers ya comprimidos.
- `key_manager`: prepara solicitud de password, derivacion de llave temporal y destruccion segura.
- `secure_memory`: centraliza `secure_zero` para borrar material sensible.
- `file_format`: define `SecureFileHeader` con magic, version, algoritmos, tamanos, salt y nonce.
- `io_pipeline`: coordina el flujo guardar/cargar sin conocer detalles internos de compresion o cifrado.
- `profiler`: prepara tiempos separados por compresion, cifrado, escritura y total.
- `tui`: queda como punto futuro de integracion del editor y ncurses.

## Reglas De Integracion

- La compresion y el cifrado trabajan sobre `Buffer`, no sobre archivos.
- El pipeline es el unico modulo que debe coordinar compresion, cifrado, formato de archivo y profiling.
- Las llaves se entregan al modulo crypto mediante `CryptoContext` y se limpian con `key_manager_destroy_key`.
- El formato de archivo no debe ejecutar I/O; solo define y valida metadatos.
