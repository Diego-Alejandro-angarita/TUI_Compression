# Repository Guidelines

## Project Structure & Module Organization

This is a C project for a terminal UI that edits text, compresses/decompresses files with Zstd, and reports execution statistics. Source files live in `src/`; matching internal interfaces live in `include/`. Keep module boundaries clear:

- `src/tui.c`, `src/editor.c`: ncurses UI and in-memory text editing.
- `src/compression.c`: Zstd compression and decompression logic.
- `src/file_io.c`: file reading and writing.
- `src/stats.c`: statistics collection and reporting helpers.
- `tests/`: future test programs or fixtures.
- `data/`: sample input files.
- `docs/`: development notes and design decisions.
- `build/`: generated objects and binaries; do not commit build outputs.

## Build, Test, and Development Commands

- `make build`: compile the project without external link flags.
- `make build-with-deps`: clean and build while linking `ncurses` and `zstd`.
- `make run`: build and run `build/zstd_tui`.
- `make test`: placeholder target; add real tests here as they are implemented.
- `make valgrind`: run the TUI under Valgrind for leak checks.
- `make clean`: remove `build/`.

Expected system packages include `gcc`, `make`, `libncurses-dev`, `libzstd-dev`, and `valgrind`.

## Coding Style & Naming Conventions

Use C11 and keep code warning-clean under `-Wall -Wextra -Wpedantic`. Follow the existing style: 4-space indentation, braces on the next line for functions, and snake_case for functions, variables, files, and struct fields. Public module declarations belong in `include/*.h`; implementation details should stay `static` inside the corresponding `src/*.c` file. Use clear enum result codes for recoverable errors.

## Testing Guidelines

There is no test framework yet. When adding tests, place them under `tests/` and wire them into `make test`. Prefer focused tests for file I/O, compression round trips, editor buffer operations, and stats calculations. Name test files by module, for example `tests/test_file_io.c`.

## Commit & Pull Request Guidelines

Recent history uses short Spanish summaries and one conventional commit style entry such as `feat: implementacion del editor y TUI base en ncurses`. Keep commits concise and imperative; use prefixes like `feat:`, `fix:`, or `docs:` when helpful. Pull requests should describe the changed modules, list manual commands run, mention dependency assumptions, and include screenshots or terminal notes for TUI-visible changes.

## Agent-Specific Instructions

Respect existing user changes and avoid unrelated refactors. Before integrating changes, run at least `make build`; use `make build-with-deps` and `make valgrind` when touching ncurses, Zstd, dynamic memory, or file handling.
