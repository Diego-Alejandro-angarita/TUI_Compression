// src/tui.c
#include "tui.h"
#include "editor.h"
#include "file_io.h"
#include "compression.h"
#include <inttypes.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_DIR "data/"
#define DATA_DIR_LEN 5

static int build_data_path(const char *name, char *out, size_t out_size)
{
    const char *safe_name = name;
    int written;

    if (!name || !*name || !out || out_size == 0) {
        return 0;
    }

    if (strncmp(name, DATA_DIR, DATA_DIR_LEN) == 0) {
        safe_name = name + DATA_DIR_LEN;
    }

    if (!*safe_name || strstr(safe_name, "..") ||
        strchr(safe_name, '/') || strchr(safe_name, '\\')) {
        return 0;
    }

    written = snprintf(out, out_size, "%s%s", DATA_DIR, safe_name);
    return written >= 0 && (size_t)written < out_size;
}

static int append_suffix(const char *path, const char *suffix, char *out, size_t out_size)
{
    int written;

    if (!path || !suffix || !out || out_size == 0) {
        return 0;
    }

    written = snprintf(out, out_size, "%s%s", path, suffix);
    return written >= 0 && (size_t)written < out_size;
}

static void draw_menu(void)
{
    attron(A_REVERSE);
    mvprintw(0, 0, " F2: Guardar | F3: Abrir | F4: Comprimir | F5: Descomprimir | F6: Stats | F10: Salir ");
    for (int i = 100; i < COLS; i++) mvaddch(0, i, ' ');
    attroff(A_REVERSE);
}

static void show_message(const char *msg)
{
    mvprintw(LINES - 1, 0, "%s (Presione una tecla)", msg);
    clrtoeol();
    refresh();
    getch();
    move(LINES - 1, 0);
    clrtoeol();
}

static void prompt_input(const char *prompt, char *buffer, int max_len)
{
    mvprintw(LINES - 1, 0, "%s", prompt);
    clrtoeol();
    echo();
    curs_set(1);
    getnstr(buffer, max_len);
    noecho();
    move(LINES - 1, 0);
    clrtoeol();
}

int tui_run(AppState *state)
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(1);

    TextEditor editor;
    editor_init(&editor);

    if (state->text_buffer)
        editor_load_text(&editor, state->text_buffer);

    int running = 1;
    char filepath[256] = {0};

    while (running) {
        clear();
        draw_menu();

        mvprintw(2, 0, "%s", editor.buffer ? editor.buffer : "");

        int row = 2, col = 0;
        for (size_t i = 0; i < editor.cursor && editor.buffer; i++) {
            if (editor.buffer[i] == '\n') {
                row++;
                col = 0;
            } else {
                col++;
            }
        }

        if (row >= LINES - 2) row = LINES - 2;
        if (col >= COLS - 1) col = COLS - 1;

        move(row, col);
        refresh();

        int ch = getch();

        switch (ch) {

            case KEY_F(10):
                running = 0;
                break;

            case KEY_F(2): {
                prompt_input("Guardar como (en data/): ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {
                    FileIoResult res = file_io_write_all(filepath, editor.buffer, editor.length);
                    if (res == FILE_IO_OK) show_message("Archivo guardado");
                    else show_message("Error al guardar");
                }
                break;
            }

            case KEY_F(3): {
                prompt_input("Abrir archivo (en data/): ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {
                    char *new_buf = NULL;
                    size_t len = 0;
                    FileIoResult res = file_io_read_all(filepath, &new_buf, &len);
                    if (res == FILE_IO_OK) {
                        editor_load_text(&editor, new_buf);
                        free(new_buf);
                        show_message("Archivo cargado");
                    } else {
                        show_message("Error al abrir");
                    }
                }
                break;
            }

            case KEY_F(4): {
                prompt_input("Archivo a comprimir (en data/): ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {

                    char input_path[512];
                    char outpath[512];

                    if (!build_data_path(filepath, input_path, sizeof(input_path)) ||
                        !append_suffix(input_path, ".zst", outpath, sizeof(outpath))) {
                        show_message("Nombre de archivo invalido");
                        break;
                    }

                    stats_report_init(&state->last_stats);
                    CompressionResult res =
                        compression_compress_file(input_path, outpath, &state->last_stats);

                    if (res == COMPRESSION_OK) {
                        stats_finalize(&state->last_stats);
                        show_message("Compresion exitosa");
                    } else {
                        show_message("Error de compresion");
                    }
                }
                break;
            }

            case KEY_F(5): {
                prompt_input("Archivo a descomprimir (en data/): ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {

                    char input_path[512];
                    char outpath[512];

                    if (!build_data_path(filepath, input_path, sizeof(input_path)) ||
                        !append_suffix(input_path, ".out", outpath, sizeof(outpath))) {
                        show_message("Nombre de archivo invalido");
                        break;
                    }

                    stats_report_init(&state->last_stats);
                    CompressionResult res =
                        compression_decompress_file(input_path, outpath, &state->last_stats);

                    if (res == COMPRESSION_OK) {
                        stats_finalize(&state->last_stats);
                        show_message("Descompresion exitosa");
                    } else {
                        show_message("Error de descompresion");
                    }
                }
                break;
            }

            case KEY_F(6):
                tui_show_stats(state);
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                editor_delete_char(&editor);
                break;

            case KEY_LEFT:
                if (editor.cursor > 0) editor.cursor--;
                break;

            case KEY_RIGHT:
                if (editor.cursor < editor.length) editor.cursor++;
                break;

            default:
                if (ch >= 32 && ch <= 126) {
                    editor_insert_char(&editor, (char)ch);
                } else if (ch == '\n' || ch == KEY_ENTER) {
                    editor_insert_char(&editor, '\n');
                }
                break;
        }
    }

    editor_destroy(&editor);
    editor_free_history();
    endwin();
    return 0;
}

void tui_show_stats(const AppState *state)
{
    const StatsReport *stats = &state->last_stats;

    clear();
    mvprintw(2, 2, "STATS:");
    mvprintw(4, 4, "Bytes originales: %" PRIu64, stats->bytes_original);
    mvprintw(5, 4, "Bytes comprimidos: %" PRIu64, stats->bytes_compressed);
    mvprintw(6, 4, "Bytes escritos: %" PRIu64, stats->bytes_written);
    mvprintw(7, 4, "Llamadas write(): %" PRIu64, stats->write_calls);
    mvprintw(9, 4, "CPU usuario: %.2f ms", stats->cpu_user_ms);
    mvprintw(10, 4, "CPU sistema: %.2f ms", stats->cpu_sys_ms);
    mvprintw(11, 4, "Tiempo total: %.2f ms", stats->wall_clock_ms);
    mvprintw(12, 4, "Ratio compresion: %.4f", stats->compression_ratio);
    mvprintw(14, 2, "Presione una tecla...");
    refresh();
    getch();
}
