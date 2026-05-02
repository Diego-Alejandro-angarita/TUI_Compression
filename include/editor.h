// include/editor.h
#ifndef EDITOR_H
#define EDITOR_H

#include <stddef.h>

/* ── Estructura del editor de texto en memoria ── */
typedef struct {
    char   *buffer;
    size_t  length;
    size_t  capacity;
    size_t  cursor;
} TextEditor;

void editor_init(TextEditor *editor);
void editor_destroy(TextEditor *editor);
int  editor_load_text(TextEditor *editor, const char *text);
int  editor_insert_char(TextEditor *editor, char value);
int  editor_delete_char(TextEditor *editor);

/* ── Sesión de entrada interactiva (readline / fallback) ──
   Lee líneas hasta que el usuario escriba "EOF" o pulse Ctrl-D.
   Devuelve texto acumulado (heap); el llamador debe liberar con free(). */
char *editor_read_session(const char *prompt_base);

/* Libera la memoria interna del historial (llamar al cerrar). */
void  editor_free_history(void);

#endif /* EDITOR_H */