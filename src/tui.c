#include "tui.h"
#include "editor.h"
#include "file_io.h"
#include "compression.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

// Dibuja el menú en la parte superior
static void draw_menu() {
    attron(A_REVERSE);
    mvprintw(0, 0, " F2: Guardar | F3: Abrir | F4: Comprimir | F5: Descomprimir | F6: Stats | F10: Salir ");
    // Rellenar el resto de la línea con el fondo invertido
    for (int i = 84; i < COLS; i++) mvaddch(0, i, ' ');
    attroff(A_REVERSE);
}

// Muestra un mensaje en la última línea de la pantalla
static void show_message(const char *msg) {
    mvprintw(LINES - 1, 0, "%s (Presione cualquier tecla para continuar...)", msg);
    clrtoeol();
    refresh();
    getch();
    // Limpiar la línea después
    move(LINES - 1, 0);
    clrtoeol();
}

// Pide un texto al usuario (ej: nombre de archivo) en la última línea
static void prompt_input(const char *prompt, char *buffer, int max_len) {
    mvprintw(LINES - 1, 0, "%s", prompt);
    clrtoeol();
    echo();
    curs_set(1);
    getnstr(buffer, max_len);
    noecho();
    
    // Limpiar la línea después
    move(LINES - 1, 0);
    clrtoeol();
}

int tui_run(AppState *state) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(1);

    TextEditor editor;
    editor_init(&editor);

    if (state->text_buffer) {
        editor_load_text(&editor, state->text_buffer);
    }

    int running = 1;
    char filepath[256] = {0};

    while (running) {
        clear();
        draw_menu();

        // Mostrar texto (empezando en la línea 2)
        mvprintw(2, 0, "%s", editor.buffer);

        // Posicionar cursor lógico
        move(2, editor.cursor);
        refresh();

        int ch = getch();

        switch (ch) {
            case KEY_F(10): // Salir
                running = 0;
                break;
            
            case KEY_F(2): // Guardar
                prompt_input("Guardar como (ruta): ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {
                    FileIoResult res = file_io_write_all(filepath, editor.buffer, editor.length);
                    if (res == FILE_IO_OK) show_message("Archivo guardado exitosamente.");
                    else show_message("Error: No se pudo escribir el archivo.");
                    strcpy(state->current_path, filepath);
                }
                break;

            case KEY_F(3): // Abrir
                prompt_input("Abrir archivo (ruta): ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {
                    char *new_buf = NULL;
                    size_t new_len = 0;
                    FileIoResult res = file_io_read_all(filepath, &new_buf, &new_len);
                    if (res == FILE_IO_OK) {
                        editor_load_text(&editor, new_buf);
                        free(new_buf);
                        show_message("Archivo cargado.");
                        strcpy(state->current_path, filepath);
                    } else {
                        show_message("Error: No se pudo leer el archivo.");
                    }
                }
                break;

            case KEY_F(4): // Comprimir
                prompt_input("Archivo a comprimir: ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {
                    char outpath[256];
                    snprintf(outpath, sizeof(outpath), "%s.zst", filepath);
                    CompressionResult res = compression_compress_file(filepath, outpath, &state->last_stats);
                    if (res == COMPRESSION_OK) show_message("Archivo comprimido.");
                    else show_message("Error de compresion (Zstd pendiente o fallo).");
                }
                break;

            case KEY_F(5): // Descomprimir
                prompt_input("Archivo a descomprimir: ", filepath, sizeof(filepath) - 1);
                if (strlen(filepath) > 0) {
                    char outpath[256];
                    snprintf(outpath, sizeof(outpath), "%s.out", filepath);
                    CompressionResult res = compression_decompress_file(filepath, outpath, &state->last_stats);
                    if (res == COMPRESSION_OK) show_message("Archivo descomprimido.");
                    else show_message("Error de descompresion (Zstd pendiente o fallo).");
                }
                break;

            case KEY_F(6): // Stats
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

    if (state->text_buffer) free(state->text_buffer);
    state->text_buffer = strdup(editor.buffer);
    state->text_length = editor.length;

    editor_destroy(&editor);
    endwin();
    return 0;
}

void tui_show_stats(const AppState *state) {
    clear();
    attron(A_BOLD);
    mvprintw(1, 2, "--- Estadisticas de Ejecucion ---");
    attroff(A_BOLD);

    const StatsReport *s = &state->last_stats;
    mvprintw(3, 4, "Volumen Clasico:    %zu bytes", s->bytes_written_classic);
    mvprintw(4, 4, "Volumen Propuesto:  %zu bytes", s->bytes_written_proposed);
    mvprintw(6, 4, "Llamadas write() (Clasico):   %zu", s->write_calls_classic);
    mvprintw(7, 4, "Llamadas write() (Propuesto): %zu", s->write_calls_proposed);
    mvprintw(9, 4, "Tiempo Wall-clock:  %.2f ms", s->wall_time_ms);
    mvprintw(10, 4, "Tiempo CPU (User):  %.2f ms", s->user_time_ms);
    mvprintw(11, 4, "Tiempo SO (Sys):    %.2f ms", s->system_time_ms);

    mvprintw(14, 2, "Presione cualquier tecla para volver al editor...");
    refresh();
    getch();
}