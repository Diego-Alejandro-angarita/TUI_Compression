# Secure Compression Pipeline Stub

Proyecto C11 para preparar una arquitectura de editor/TUI con pipeline de texto plano, compresion ZSTD, cifrado simetrico y escritura segura.

Esta etapa solo contiene esqueletos compilables. No implementa compresion real, cifrado real, lectura/escritura real ni TUI con ncurses.

## Pipeline Objetivo

Guardar:

```text
texto plano en memoria -> compresion -> cifrado simetrico -> escritura a disco
```

Cargar:

```text
lectura desde disco -> descifrado -> descompresion -> texto plano recuperado
```

## Comandos

```sh
make
./bin/editor_secure_stub
make test
make clean
```

El binario debe imprimir:

```text
Secure compression pipeline structure initialized successfully.
```

## Estructura

- `include/`: interfaces publicas internas por modulo.
- `src/`: implementaciones stub y utilidades simples.
- `tests/`: pruebas basicas de estructura y stubs.
- `docs/`: decisiones de arquitectura, pipeline, profiling y reparto del equipo.
- `data/`: datos de ejemplo heredados del proyecto original.

## Estado Actual

- `buffer`: buffer generico en memoria.
- `compression`: interfaz futura para ZSTD a nivel de buffer.
- `crypto`: interfaz futura para cifrado simetrico a nivel de buffer.
- `key_manager`: estructura para password, derivacion de llave y borrado seguro.
- `secure_memory`: borrado seguro simple con puntero `volatile`.
- `file_format`: header logico del archivo seguro.
- `io_pipeline`: orquestacion del flujo guardar/cargar sin I/O real.
- `profiler`: API preparada para tiempos separados.
- `tui`: stub sin `ncurses`.
