#include "tui.h"
#include "editor.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

// Helper para dibujar el menú superior
void draw_menu() {
    attron(A_REVERSE);
    mvprintw(0, 0, " F2: Guardar | F3: Abrir | F4: Comprimir | F5: Descomprimir | F6: Stats | F10: Salir ");
    attroff(A_REVERSE);
}

int tui_run(AppState *state) {
    initscr();
    raw();              // Desactiva el buffering de línea
    keypad(stdscr, TRUE); // Permite usar F1, F2, flechas, etc.
    noecho();           // No muestra las teclas presionadas automáticamente
    cursor_set(1);      // Cursor visible

    TextEditor editor;
    editor_init(&editor);

    // Si ya hay texto en el estado (por ejemplo, de un archivo abierto), cargarlo
    if (state->text_buffer) {
        editor_load_text(&editor, state->text_buffer);
    }

    int ch;
    int running = 1;

    while (running) {
        clear();
        draw_menu();

        // Dibujar el contenido del editor (empezando en la línea 2)
        mvprintw(2, 0, "%s", editor.buffer);

        // Posicionar el cursor visual donde está el cursor lógico
        // (Nota: Esto es una simplificación para texto en una sola línea)
        move(2, editor.cursor); 
        
        refresh();

        ch = getch();

        switch (ch) {
            case KEY_F(10): // Salir
                running = 0;
                break;
            
            case KEY_F(6): // Mostrar Estadísticas
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
                // Solo insertar caracteres imprimibles
                if (ch >= 32 && ch <= 126) {
                    editor_insert_char(&editor, (char)ch);
                } else if (ch == '\n' || ch == KEY_ENTER) {
                    editor_insert_char(&editor, '\n');
                }
                break;
        }
    }

    // Al salir, guardamos el buffer final en el AppState
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
    mvprintw(6, 4, "Tiempo Wall-clock:  %.2f ms", s->wall_time_ms);
    mvprintw(7, 4, "Tiempo CPU (User):  %.2f ms", s->user_time_ms);

    mvprintw(10, 2, "Presione cualquier tecla para volver al editor...");
    refresh();
    getch();
}