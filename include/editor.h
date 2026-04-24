#ifndef EDITOR_H
#define EDITOR_H

#include <stddef.h>

typedef struct {
    char *buffer;
    size_t length;
    size_t capacity;
    size_t cursor;
} TextEditor;

void editor_init(TextEditor *editor);
void editor_destroy(TextEditor *editor);
int editor_load_text(TextEditor *editor, const char *text);
int editor_insert_char(TextEditor *editor, char value);
int editor_delete_char(TextEditor *editor);

#endif

