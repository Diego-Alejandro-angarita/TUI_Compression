// src/tui.c
#include "tui.h"
#include "editor.h"
#include "file_io.h"
#include "compression.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

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
                prompt_input("Archivo a comprimir: ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {

                    char outpath[300];
                    snprintf(outpath, sizeof(outpath), "%s.zst", filepath);

                    CompressionResult res =
                        compression_compress_file(filepath, outpath, &state->last_stats);

                    if (res == COMPRESSION_OK) show_message("Compresion exitosa");
                    else show_message("Error de compresion");
                }
                break;
            }

            case KEY_F(5): {
                prompt_input("Archivo a descomprimir: ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {

                    char outpath[300];
                    snprintf(outpath, sizeof(outpath), "%s.out", filepath);

                    CompressionResult res =
                        compression_decompress_file(filepath, outpath, &state->last_stats);

                    if (res == COMPRESSION_OK) show_message("Descompresion exitosa");
                    else show_message("Error de descompresion");
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
    clear();
    mvprintw(2, 2, "STATS:");
    mvprintw(4, 4, "Bytes clasico: %zu", state->last_stats.bytes_written_classic);
    mvprintw(5, 4, "Bytes propuesto: %zu", state->last_stats.bytes_written_proposed);
    mvprintw(7, 4, "Tiempo total: %.2f ms", state->last_stats.wall_time_ms);
    mvprintw(9, 2, "Presione una tecla...");
    refresh();
    getch();
}