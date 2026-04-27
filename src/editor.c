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
    editor->cursor = new_len; // Cursor al final
    return 0;
}

int editor_insert_char(TextEditor *editor, char value) {
    if (!editor || !editor->buffer) return -1;

    // Redimensionar si es necesario
    if (editor->length + 1 >= editor->capacity) {
        size_t new_cap = editor->capacity * 2;
        char *new_buf = realloc(editor->buffer, new_cap);
        if (!new_buf) return -1;
        editor->buffer = new_buf;
        editor->capacity = new_cap;
    }

    // Desplazar texto a la derecha para insertar
    memmove(&editor->buffer[editor->cursor + 1], 
            &editor->buffer[editor->cursor], 
            editor->length - editor->cursor + 1);

    editor->buffer[editor->cursor] = value;
    editor->length++;
    editor->cursor++;
    
    return 0;
}

int editor_delete_char(TextEditor *editor) {
    if (!editor || editor->cursor == 0) return -1;

    // Desplazar texto a la izquierda para borrar (comportamiento Backspace)
    memmove(&editor->buffer[editor->cursor - 1], 
            &editor->buffer[editor->cursor], 
            editor->length - editor->cursor + 1);

    editor->length--;
    editor->cursor--;
    
    return 0;
}