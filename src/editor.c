#include "editor.h"

void editor_init(TextEditor *editor)
{
    if (editor == NULL) {
        return;
    }

    editor->buffer = 0;
    editor->length = 0;
    editor->capacity = 0;
    editor->cursor = 0;
}

void editor_destroy(TextEditor *editor)
{
    if (editor == NULL) {
        return;
    }

    editor->buffer = 0;
    editor->length = 0;
    editor->capacity = 0;
    editor->cursor = 0;
}

int editor_load_text(TextEditor *editor, const char *text)
{
    (void)editor;
    (void)text;

    return -1;
}

int editor_insert_char(TextEditor *editor, char value)
{
    (void)editor;
    (void)value;

    return -1;
}

int editor_delete_char(TextEditor *editor)
{
    (void)editor;

    return -1;
}

