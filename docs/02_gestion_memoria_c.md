# Gestion de Memoria en C

Este documento explica como se organizan las estructuras y como se controla el ciclo de vida de la memoria dinamica en la aplicacion.

## Estructuras principales

### `TextEditor`

Definida en `include/editor.h`:

```c
typedef struct {
    char   *buffer;
    size_t  length;
    size_t  capacity;
    size_t  cursor;
} TextEditor;
```

Diseno:

1. `buffer` apunta al texto reservado en heap.
2. `length` guarda bytes usados, sin contar el terminador `\0`.
3. `capacity` guarda bytes reservados.
4. `cursor` guarda la posicion lineal del cursor.

Los campos estan agrupados por alineacion natural: un puntero y tres `size_t`. En plataformas de 64 bits todos suelen alinear a 8 bytes, por lo que no se introducen huecos internos relevantes por `padding`.

### `AppState`

Definida en `include/app_state.h`:

```c
typedef struct {
    char *text_buffer;
    size_t text_length;
    char current_path[256];
    StatsReport last_stats;
} AppState;
```

Diseno:

1. Mantiene el estado compartido entre `main` y la TUI.
2. `current_path` usa un arreglo fijo de 256 bytes para evitar asignaciones dinamicas por ruta.
3. `last_stats` queda embebido dentro de la estructura, evitando otro puntero y otra reserva heap.

Puede existir alineacion antes de `StatsReport` dependiendo de la plataforma, pero el impacto es bajo frente al arreglo fijo de ruta. No se usa `packed` porque podria generar accesos no alineados y empeorar rendimiento.

### `StatsReport`

Definida en `include/stats.h`:

```c
typedef struct {
    uint64_t bytes_original;
    uint64_t bytes_compressed;
    uint64_t bytes_written;
    uint64_t write_calls;
    double cpu_user_ms;
    double cpu_sys_ms;
    double wall_clock_ms;
    double compression_ratio;
} StatsReport;
```

Diseno:

1. Agrupa primero contadores `uint64_t`.
2. Agrupa despues mediciones `double`.
3. Ambos tipos suelen tener alineacion de 8 bytes, por lo que el orden evita padding interno innecesario.
4. Se pasa por puntero a los modulos para acumular datos sin copias grandes.

### `StatsTimer`

Definida en `include/stats.h`:

```c
typedef struct {
    double wall_start_ms;
    double cpu_user_start_ms;
    double cpu_sys_start_ms;
} StatsTimer;
```

Solo contiene `double`, por lo que no presenta padding interno relevante.

### `WBuf`

Definida como estructura privada en `src/compression.c`:

```c
typedef struct {
    int      fd;
    uint8_t  data[4096];
    size_t   used;
    uint64_t write_calls;
    uint64_t total_written;
} WBuf;
```

Diseno:

1. Es privada del modulo de compresion.
2. `data[4096]` evita reservar y liberar un buffer heap por cada escritura.
3. `used`, `write_calls` y `total_written` permiten medir la salida real.
4. Puede haber padding despues de `fd` o antes de campos alineados, pero el costo es minimo frente a los 4096 bytes del buffer.

## Ciclo de vida de memoria

| Recurso | Reserva | Uso | Liberacion |
| --- | --- | --- | --- |
| `TextEditor.buffer` | `editor_init` con `malloc` | Edicion, insercion, borrado y carga de texto | `editor_destroy` |
| Buffer cargado desde archivo | `file_io_read_all` con `malloc` | La TUI lo copia al editor con `editor_load_text` | `free(new_buf)` en `tui.c` |
| Historial fallback | `hist_add` con `realloc` y duplicados heap | Navegacion con flechas en entrada manual | `editor_free_history` |
| `line_buf` | `compression_compress_file` con `malloc` | Lectura de una linea fuente | Bloque `done` antes de retornar |
| `comp_buf` | `compression_compress_file` con `malloc` | Frame Zstd comprimido | Bloque `done` antes de retornar |
| `frame_buf` | `compression_decompress_file` con `malloc` y `realloc` | Frame comprimido leido | Bloque `done_d` antes de retornar |
| `decomp_buf` | `compression_decompress_file` con `malloc` y `realloc` | Bytes descomprimidos | Bloque `done_d` antes de retornar |
| `WBuf.data` | Automatico en stack | Acumula 4096 bytes | Sale de alcance automaticamente |
| `AppState.text_buffer` | Actualmente inicia en `NULL` | Reservado para estado compartido futuro | `app_state_destroy` |

## Estrategias contra fugas

1. Cada modulo expone una funcion de destruccion cuando administra memoria persistente: `editor_destroy`, `editor_free_history` y `app_state_destroy`.
2. Las funciones de compresion usan etiquetas de salida unica (`done`, `done_d`) para liberar buffers incluso cuando ocurre un error intermedio.
3. Las rutas de error tras `malloc` parcial liberan los punteros que si alcanzaron a reservarse.
4. El patron de `realloc` usa un puntero temporal antes de reemplazar el puntero original. Si `realloc` falla, el buffer anterior no se pierde.
5. Los archivos abiertos con `fopen` y descriptores abiertos con `open` se cierran con `fclose` y `close` antes de retornar.
6. `stats_init` y `app_state_init` inicializan memoria estructural con cero o valores nulos para evitar lecturas de campos indeterminados.

## Decisiones sobre padding

No se fuerza empaquetado con atributos como `__attribute__((packed))`. La prioridad es mantener alineacion natural para que CPU y compilador generen accesos eficientes. La reduccion de desperdicio se logra ordenando campos compatibles y evitando estructuras publicas con mezclas innecesarias de `char`, `int`, punteros y `double` intercalados.

## Validacion recomendada

Para verificar fugas y errores de memoria, ejecutar:

```sh
make valgrind
```

El objetivo esperado es que Valgrind no reporte bloques definitivamente perdidos ni accesos invalidos durante el ciclo de vida normal de la aplicacion.
