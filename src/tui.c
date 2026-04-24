#include "tui.h"

#if defined(__has_include)
#if __has_include(<ncurses.h>)
#define TUI_HAS_NCURSES 1
#include <ncurses.h>
#endif
#endif

#ifndef TUI_HAS_NCURSES
#include <stdio.h>
#endif

int tui_run(AppState *state)
{
    (void)state;

#ifdef TUI_HAS_NCURSES
    initscr();
    cbreak();
    noecho();

    printw("TUI Zstd - estructura inicial\n");
    printw("Funcionalidades pendientes: editor, compresion, descompresion y estadisticas.\n");
    printw("Presione cualquier tecla para salir.\n");
    refresh();
    getch();

    endwin();
#else
    puts("TUI Zstd - estructura inicial");
    puts("Funcionalidades pendientes: editor, compresion, descompresion y estadisticas.");
    puts("Instale ncurses para desarrollar la TUI real.");
#endif
    return 0;
}

void tui_show_stats(const AppState *state)
{
    (void)state;
}
