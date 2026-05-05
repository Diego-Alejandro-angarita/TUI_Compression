# TUI de Compresion Zstd

Proyecto C para una TUI que permita crear, editar texto sencillo,
comprimir linea por linea y descomprimir archivos sin perdida usando Zstd.

El proyecto implementa lectura/escritura de archivos, compresion y
descompresion con Zstd, estadisticas basicas de ejecucion y una interfaz TUI.

## Dependencias esperadas

No se instalan desde este proyecto. En el sistema de desarrollo se espera contar con:

- `gcc`
- `make`
- `libncurses-dev`
- `libzstd-dev`
- `valgrind`

## Comandos

```sh
make build
make run
make build-with-deps
make valgrind
make clean
```

`make build` compila los objetos del proyecto. `make build-with-deps` limpia y
construye el binario final enlazando contra `ncurses` y `zstd` cuando esas
dependencias ya esten instaladas.

## Cuando es util comprimir linea por linea

La compresion linea por linea no tiene un umbral universal basado solo en el
tamano total del archivo. En este proyecto cada linea se guarda como un frame
Zstd independiente con un encabezado propio de 4 bytes, por lo que el resultado
depende mucho de la longitud promedio de las lineas y de cuanto se repiten sus
patrones internos.

Como regla practica, empieza a ser realmente util desde archivos de texto de
aproximadamente 64 KB cuando las lineas son medianas o largas, por ejemplo logs,
CSV o JSONL con lineas de 512 bytes o mas. Para archivos con lineas cortas, como
texto normal de 40 a 120 bytes por linea, conviene usarla solo desde alrededor
de 1 MB y cuando se necesite procesar o recuperar el contenido por lineas. Por
debajo de 16 KB normalmente no compensa: el costo de un frame por linea puede
igualar o superar el ahorro de Zstd.

| Escenario | Tamano donde empieza a convenir | Resultado esperado | Recomendacion |
| --- | ---: | --- | --- |
| Texto pequeno con lineas cortas | Menos de 16 KB | Puede crecer por overhead de frames | Evitar linea por linea |
| Texto comun, lineas de 40 a 120 bytes | 1 MB o mas | Ahorro moderado, peor que comprimir el archivo completo | Usar solo si se necesita procesar por lineas |
| Logs, CSV o JSONL con lineas de 512 bytes o mas | 64 KB a 256 KB | Buen ahorro con lectura/descompresion incremental | Buen caso de uso |
| Archivos grandes, 10 MB o mas, con registros independientes | 10 MB o mas | Util para streaming y recuperacion parcial | Recomendado si importa el acceso por registro |
| Archivo completo comprimido con Zstd | Desde pocos KB si hay repeticion | Mejor ratio porque Zstd ve patrones entre lineas | Preferir si no hace falta acceso linea por linea |

En resumen: si el objetivo principal es obtener el archivo mas pequeno posible,
comprimir el archivo completo suele ganar. La compresion linea por linea es util
cuando se valora descomprimir por partes, medir registros independientes o evitar
cargar todo el archivo en memoria.

## Estructura

- `src/`: implementaciones en C.
- `include/`: interfaces internas del proyecto.
- `tests/`: pruebas futuras.
- `docs/`: documentacion de desarrollo.
- `data/`: archivos de ejemplo.
- `build/`: archivos generados por compilacion.
