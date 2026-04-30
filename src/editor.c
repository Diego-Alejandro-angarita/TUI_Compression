#include "editor.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 1024

void editor_init(TextEditor *editor) {
    if (!editor) return;
    editor->capacity = INITIAL_CAPACITY;
    editor->length = 0;
    editor->cursor = 0;
    editor->buffer = malloc(editor->capacity);
    if (editor->buffer) {
        editor->buffer[0] = '\0';
    }
}

void editor_destroy(TextEditor *editor) {
    if (!editor) return;
    if (editor->buffer) {
        free(editor->buffer);
        editor->buffer = NULL;
    }
    editor->length = 0;
    editor->capacity = 0;
    editor->cursor = 0;
}

int editor_load_text(TextEditor *editor, const char *text) {
    if (!editor || !text) return -1;
    
    size_t new_len = strlen(text);
    if (new_len >= editor->capacity) {
        size_t new_cap = new_len + INITIAL_CAPACITY;
        char *new_buf = realloc(editor->buffer, new_cap);
        if (!new_buf) return -1;
        editor->buffer = new_buf;
        editor->capacity = new_cap;
    }
    
    strcpy(editor->buffer, text);
    editor->length = new_len;
    editor->cursor = new_len; // Coloca el cursor al final del texto cargado
    return 0;
}

int editor_insert_char(TextEditor *editor, char value) {
    if (!editor || !editor->buffer) return -1;

    // Verificar si necesitamos más espacio
    if (editor->length + 1 >= editor->capacity) {
        size_t new_cap = editor->capacity * 2;
        char *new_buf = realloc(editor->buffer, new_cap);
        if (!new_buf) return -1;
        editor->buffer = new_buf;
        editor->capacity = new_cap;
    }

    // Desplazar el texto a la derecha para hacer espacio al nuevo carácter
    memmove(&editor->buffer[editor->cursor + 1], 
            &editor->buffer[editor->cursor], 
            editor->length - editor->cursor + 1);

    editor->buffer[editor->cursor] = value;
    editor->length++;
    editor->cursor++;
    
    return 0;
}

int editor_delete_char(TextEditor *editor) {
    // Comportamiento de Backspace: borra el carácter a la izquierda del cursor
    if (!editor || editor->cursor == 0) return -1;

    // Desplazar el texto a la izquierda para cubrir el carácter borrado
    memmove(&editor->buffer[editor->cursor - 1], 
            &editor->buffer[editor->cursor], 
            editor->length - editor->cursor + 1);

    editor->length--;
    editor->cursor--;
    
    return 0;
}