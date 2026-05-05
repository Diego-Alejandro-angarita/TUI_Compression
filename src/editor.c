// src/editor.c
#include "editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 1024
#define LINE_MAX_LEN     1024
#define SESSION_BUF_INIT 4096

static char *editor_strdup(const char *text)
{
    if (!text) return NULL;

    size_t len = strlen(text) + 1;
    char *copy = malloc(len);
    if (!copy) return NULL;

    memcpy(copy, text, len);
    return copy;
}

/* ══════════════════════════════════════════════════════════
   Funciones originales del editor en memoria
   ══════════════════════════════════════════════════════════ */

void editor_init(TextEditor *editor)
{
    if (!editor) return;
    editor->capacity = INITIAL_CAPACITY;
    editor->length   = 0;
    editor->cursor   = 0;
    editor->buffer   = malloc(editor->capacity);
    if (editor->buffer)
        editor->buffer[0] = '\0';
}

void editor_destroy(TextEditor *editor)
{
    if (!editor) return;
    free(editor->buffer);
    editor->buffer   = NULL;
    editor->length   = 0;
    editor->capacity = 0;
    editor->cursor   = 0;
}

int editor_load_text(TextEditor *editor, const char *text)
{
    if (!editor || !text) return -1;

    size_t new_len = strlen(text);
    if (new_len >= editor->capacity) {
        size_t new_cap = new_len + INITIAL_CAPACITY;
        char  *new_buf = realloc(editor->buffer, new_cap);
        if (!new_buf) return -1;
        editor->buffer   = new_buf;
        editor->capacity = new_cap;
    }

    strcpy(editor->buffer, text);
    editor->length = new_len;
    editor->cursor = new_len;
    return 0;
}

int editor_insert_char(TextEditor *editor, char value)
{
    if (!editor || !editor->buffer) return -1;

    if (editor->length + 1 >= editor->capacity) {
        size_t new_cap = editor->capacity * 2;
        char  *new_buf = realloc(editor->buffer, new_cap);
        if (!new_buf) return -1;
        editor->buffer   = new_buf;
        editor->capacity = new_cap;
    }

    memmove(&editor->buffer[editor->cursor + 1],
            &editor->buffer[editor->cursor],
            editor->length - editor->cursor + 1);

    editor->buffer[editor->cursor] = value;
    editor->length++;
    editor->cursor++;
    return 0;
}

int editor_delete_char(TextEditor *editor)
{
    if (!editor || editor->cursor == 0) return -1;

    memmove(&editor->buffer[editor->cursor - 1],
            &editor->buffer[editor->cursor],
            editor->length - editor->cursor + 1);

    editor->length--;
    editor->cursor--;
    return 0;
}

/* ══════════════════════════════════════════════════════════
   Sesión de entrada interactiva
   ══════════════════════════════════════════════════════════ */

#ifdef HAVE_READLINE
#  include <readline/readline.h>
#  include <readline/history.h>

static char *read_one_line(const char *prompt)
{
    return readline(prompt);
}

static void hist_add(const char *line)
{
    if (line && *line) add_history(line);
}

void editor_free_history(void)
{
    clear_history();
}

#else  /* ── fallback: raw-mode manual ── */

#  ifdef __unix__
#    include <termios.h>
#    include <unistd.h>

typedef struct {
    char **entries;
    int    count;
    int    cap;
    int    pos;
} History;

static History g_hist = {NULL, 0, 0, 0};

static void hist_add(const char *line)
{
    if (!line || !*line) return;
    if (g_hist.count >= g_hist.cap) {
        int nc = g_hist.cap ? g_hist.cap * 2 : 64;
        char **tmp = realloc(g_hist.entries, sizeof(char *) * (size_t)nc);
        if (!tmp) return;
        g_hist.entries = tmp;
        g_hist.cap     = nc;
    }
    g_hist.entries[g_hist.count] = editor_strdup(line);
    if (!g_hist.entries[g_hist.count]) return;
    g_hist.count++;
    g_hist.pos = g_hist.count;
}

void editor_free_history(void)
{
    for (int i = 0; i < g_hist.count; i++) free(g_hist.entries[i]);
    free(g_hist.entries);
    g_hist.entries = NULL;
    g_hist.count   = 0;
    g_hist.cap     = 0;
    g_hist.pos     = 0;
}

static struct termios g_term_orig;

static void raw_on(void)
{
    tcgetattr(STDIN_FILENO, &g_term_orig);
    struct termios raw = g_term_orig;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

static void raw_off(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &g_term_orig);
}

static char *read_one_line(const char *prompt)
{
    char buf[LINE_MAX_LEN];
    int  len = 0;

    printf("%s", prompt);
    fflush(stdout);
    raw_on();

    while (1) {
        int c = getchar();

        if (c == EOF || c == 4) {               /* Ctrl-D */
            raw_off();
            if (len == 0) return NULL;
            buf[len] = '\0';
            break;
        }
        if (c == '\n' || c == '\r') {
            raw_off();
            buf[len] = '\0';
            putchar('\n');
            break;
        }
        if (c == 127 || c == 8) {               /* Backspace */
            if (len > 0) { len--; printf("\b \b"); fflush(stdout); }
            continue;
        }
        if (c == 27) {                           /* ESC sequence */
            int c2 = getchar();
            if (c2 == '[') {
                int c3 = getchar();
                if (c3 == 'A' && g_hist.pos > 0) {        /* ↑ */
                    g_hist.pos--;
                    for (int i = 0; i < len; i++) printf("\b \b");
                    strncpy(buf, g_hist.entries[g_hist.pos], LINE_MAX_LEN - 1);
                    buf[LINE_MAX_LEN - 1] = '\0';
                    len = (int)strlen(buf);
                    printf("%s", buf);
                    fflush(stdout);
                } else if (c3 == 'B') {                    /* ↓ */
                    for (int i = 0; i < len; i++) printf("\b \b");
                    if (g_hist.pos < g_hist.count - 1) {
                        g_hist.pos++;
                        strncpy(buf, g_hist.entries[g_hist.pos], LINE_MAX_LEN - 1);
                        buf[LINE_MAX_LEN - 1] = '\0';
                        len = (int)strlen(buf);
                        printf("%s", buf);
                        fflush(stdout);
                    } else {
                        g_hist.pos = g_hist.count;
                        len = 0;
                    }
                }
            }
            continue;
        }
        if (len < LINE_MAX_LEN - 1) {
            buf[len++] = (char)c;
            putchar(c);
            fflush(stdout);
        }
    }

    return editor_strdup(buf);
}

#  else  /* non-unix, no raw mode */

static void hist_add(const char *line) { (void)line; }
void editor_free_history(void) {}

static char *read_one_line(const char *prompt)
{
    printf("%s", prompt);
    fflush(stdout);
    char buf[LINE_MAX_LEN];
    if (!fgets(buf, sizeof(buf), stdin)) return NULL;
    size_t l = strlen(buf);
    if (l && buf[l - 1] == '\n') buf[l - 1] = '\0';
    return editor_strdup(buf);
}

#  endif /* __unix__ */
#endif   /* HAVE_READLINE */

/* ── Bucle principal de sesión ── */
char *editor_read_session(const char *prompt_base)
{
    size_t buf_sz  = SESSION_BUF_INIT;
    size_t buf_len = 0;
    char  *buffer  = malloc(buf_sz);
    if (!buffer) return NULL;
    buffer[0] = '\0';

    int line_num = 1;

    while (1) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "%s%d> ",
                 prompt_base ? prompt_base : "", line_num);

        char *line = read_one_line(prompt);
        if (!line) break;                        /* EOF / Ctrl-D */

        if (strcmp(line, "EOF") == 0) { free(line); break; }

        hist_add(line);

        size_t line_len  = strlen(line);
        size_t needed    = buf_len + line_len + 2; /* '\n' + '\0' */

        if (needed > buf_sz) {
            while (buf_sz < needed) buf_sz *= 2;
            char *tmp = realloc(buffer, buf_sz);
            if (!tmp) { free(line); free(buffer); return NULL; }
            buffer = tmp;
        }

        memcpy(buffer + buf_len, line, line_len);
        buf_len += line_len;
        buffer[buf_len++] = '\n';
        buffer[buf_len]   = '\0';

        free(line);
        line_num++;
    }

    return buffer;
}
