# TUI_Compression - Pipeline I/O de datos

```mermaid
flowchart TD
    A["Usuario en terminal"] -->|"teclas, F2-F6, F10"| B["ncurses TUI: tui_run"]
    B --> C["TextEditor en memoria"]
    C -->|"buffer, length, cursor"| B

    B -->|"F2: nombre de archivo + buffer"| D["file_io_write_all"]
    D -->|"valida nombre simple"| E["data/<nombre>"]
    E -->|"fwrite bytes del editor"| F["archivo de texto guardado"]

    B -->|"F3: nombre de archivo"| G["file_io_read_all"]
    G -->|"valida nombre simple"| H["data/<nombre>"]
    H -->|"fread archivo completo"| I["heap buffer + length"]
    I -->|"editor_load_text"| C

    B -->|"F4: nombre de archivo"| J["build_data_path + append_suffix"]
    J -->|"input=data/<nombre>, output=data/<nombre>.zst"| K["compression_compress_file"]
    K --> L["archivo comprimido .zst"]
    K --> M["StatsReport"]

    B -->|"F5: nombre de archivo"| N["build_data_path + append_suffix"]
    N -->|"input=data/<nombre>, output=data/<nombre>.out"| O["compression_decompress_file"]
    O --> P["archivo reconstruido .out"]
    O --> M

    B -->|"F6"| Q["tui_show_stats"]
    M -->|"bytes, write calls, tiempos, ratio"| Q
    Q -->|"mvprintw"| A
```

## Pipeline de compresion

Formato de salida por linea:

```text
[uint32_t frame_len][frame Zstd]
[uint32_t frame_len][frame Zstd]
...
```

```mermaid
flowchart LR
    A["data/<archivo>"] -->|"fopen rb"| B["FILE* in"]
    B -->|"fgetc hasta newline o EOF, max 1 MB"| C["line_buf"]
    C -->|"ZSTD_compress nivel 3"| D["comp_buf"]
    D -->|"csize"| E["uint32_t frame_len"]
    E -->|"4 bytes"| F["WBuf 4096 bytes"]
    D -->|"frame comprimido"| F
    F -->|"write cuando se llena o flush final"| G["data/<archivo>.zst"]

    C -->|"line_len acumulado"| H["stats.bytes_original"]
    F -->|"total_written"| I["stats.bytes_compressed + stats.bytes_written"]
    F -->|"write_calls"| J["stats.write_calls"]
    K["clock_gettime + getrusage"] -->|"wall/user/sys ms"| L["stats tiempos"]
```

I/O principal:

| Etapa | Entrada | Transformacion | Salida |
| --- | --- | --- | --- |
| TUI F4 | nombre escrito por usuario | valida que sea nombre simple y fuerza prefijo `data/` | `input_path`, `output_path=.zst` |
| Lectura | `data/<archivo>` | lectura byte a byte hasta `\n`, EOF o 1 MB | `line_buf` |
| Compresion | linea cruda | `ZSTD_compress(..., ZSTD_LEVEL=3)` | frame Zstd |
| Serializacion | frame Zstd | antepone `uint32_t frame_len` | registro binario |
| Escritura | registros binarios | bufferiza en bloques de 4096 bytes | archivo `.zst` |
| Estadisticas | tamanos, llamadas write, reloj | acumula y finaliza ratio | `StatsReport` |

## Pipeline de descompresion

```mermaid
flowchart LR
    A["data/<archivo>.zst"] -->|"fopen rb"| B["FILE* in"]
    B -->|"fread 4 bytes"| C["uint32_t frame_len"]
    C -->|"dimensiona frame_buf si hace falta"| D["frame_buf"]
    B -->|"fread frame_len bytes"| D
    D -->|"ZSTD_getFrameContentSize"| E["content_sz"]
    E -->|"dimensiona decomp_buf si hace falta"| F["decomp_buf"]
    D -->|"ZSTD_decompress"| F
    F -->|"bytes originales de la linea"| G["WBuf 4096 bytes"]
    G -->|"write + flush"| H["data/<archivo>.zst.out"]

    F -->|"dsize acumulado"| I["stats.bytes_original"]
    G -->|"total_written"| J["stats.bytes_written"]
    G -->|"write_calls"| K["stats.write_calls"]
    L["clock_gettime + getrusage"] -->|"wall/user/sys ms"| M["stats tiempos"]
```

I/O principal:

| Etapa | Entrada | Transformacion | Salida |
| --- | --- | --- | --- |
| TUI F5 | nombre escrito por usuario | valida nombre simple y antepone `data/` | `input_path`, `output_path=.out` |
| Lectura de header | archivo comprimido | `fread` de 4 bytes | `frame_len` |
| Lectura de frame | `frame_len` | `fread` del frame completo | `frame_buf` |
| Dimensionamiento | frame Zstd | `ZSTD_getFrameContentSize` y `realloc` si aplica | capacidad de salida |
| Descompresion | frame Zstd | `ZSTD_decompress` | bytes originales |
| Escritura | bytes originales | bufferiza en bloques de 4096 bytes | archivo `.out` |
| Estadisticas | tamanos, llamadas write, reloj | acumula y finaliza ratio | `StatsReport` |

## Observaciones de arquitectura I/O

- La TUI solo acepta nombres de archivo simples para operaciones seguras bajo `data/`;
  rechaza rutas con `..`, `/` o `\`.
- Guardar y abrir desde el editor usan `file_io_*`, que leen o escriben el archivo
  completo en memoria.
- Comprimir y descomprimir no usan `file_io_*`; trabajan directamente con `fopen`,
  `fgetc`/`fread`, `open` y `write`.
- El archivo comprimido no es un `.zst` monolitico: es una secuencia de frames Zstd
  independientes, cada uno con un encabezado propio de 4 bytes.
- `StatsReport.bytes_compressed` se llena durante compresion, pero durante
  descompresion queda en cero; por eso el ratio mostrado despues de descomprimir
  sera `0.0` con la implementacion actual.
