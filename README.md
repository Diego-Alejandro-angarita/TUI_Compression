# TUI de Compresion Zstd

Proyecto C para una TUI que permita crear, editar texto sencillo,
comprimir linea por linea y descomprimir archivos sin perdida usando Zstd.

Esta entrega solo contiene la estructura inicial, interfaces y stubs. No implementa
funcionalidad real de edicion, compresion, descompresion ni estadisticas.

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

`make build` compila la estructura inicial sin exigir librerias externas porque
todavia no hay funcionalidad real. `make build-with-deps` enlaza contra
`ncurses` y `zstd` cuando esas dependencias ya esten instaladas.

## Estructura

- `src/`: implementaciones en C.
- `include/`: interfaces internas del proyecto.
- `tests/`: pruebas futuras.
- `docs/`: documentacion de desarrollo.
- `data/`: archivos de ejemplo.
- `build/`: archivos generados por compilacion.
