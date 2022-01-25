#include "ini.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum {
    LINE_LENGTH = 1024
};

// References:
//
// [1] XDG Desktop Entry Specification:
//     https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s03.html
//
// [2] systemd.syntax — General syntax of systemd configuration files
//     https://www.freedesktop.org/software/systemd/man/systemd.syntax.html

static bool is_comment_character(char c)
{
    // # [1]
    // ; [2]
    return c == '#' || c == ';';
}

static bool is_section_character(char c)
{
    // All ASCII except for [ and ] and control characters. [1]
    return c != '[' && c != ']' && !iscntrl(c);
}

static bool is_key_character(char c)
{
    // A-Za-z0-9-. [1]
    return isalnum(c) || c == '-';
}

/// Determine if string contains an empty line.
/// @discussion An empty line is defined as either empty (zero length), blank (whitespace only), or starting with a comment character.
/// @see POSIX.1-2017 makefile syntax ("Comment lines").
/// @see is_comment_character
/// @return bool true if string is empty, blank, or a comment.
static bool is_empty_line(const char *p)
{
    for (; *p && !is_comment_character(*p); p++) {
        if (!isspace(*p)) {
            return false;
        }
    }
    return true;
}

static char *skip_ws(char *s)
{
    while (isspace(*s)) {
        s++;
    }
    return s;
}

/// Accept character.
/// @discussion Consume @c ch if available.
static void accept_char(FILE *fp, int ch)
{
    int c = fgetc(fp);

    if (c == EOF) {
        return;
    }

    if (c != ch) {
        ungetc(c, fp);
    }
}

/// Accept EOL characters.
/// @discussion Accept EOF, CR, LF, CRLF, LFCR as meaning end-of-line.
/// @return bool True if EOL reached.
static bool accept_eol(FILE *fp, int c)
{
    if (c == EOF) {
        return true;
    }

    if (c == '\r') {
        // Consume CR[,LF].
        accept_char(fp, '\n');
        return true;
    }

    if (c == '\n') {
        // Consume LF[,CR].
        accept_char(fp, '\r');
        return true;
    }

    return false;
}

/// Read line from file.
/// @discussion Read up to EOL or EOF.
/// @return int -ENOSPC if buffer too small, zero on EOF, or positive number of characters written to @c buf (including NUL terminator).
static int read_line(FILE *fp, size_t cap, char *buf)
{
    int len = 0;

    while (cap > 1) {
        int c = fgetc(fp);

        if (c == EOF && len == 0) {
            return 0;
        }

        if (accept_eol(fp, c)) {
            *buf = 0;
            len++;
            return len;
        }

        *buf++ = (char)c;
        len++;
        cap--;
    }

    for (;;) {
        int c = fgetc(fp);

        if (accept_eol(fp, c)) {
            break;
        }
    }

    return -ENOSPC;
}

int ini_read(const char *filename, ini_callback_t callback)
{
    FILE *fp = NULL;
    unsigned lineno = 1;
    char buffer[LINE_LENGTH];
    char line[LINE_LENGTH];
    char section[LINE_LENGTH];
    char *p = NULL;

    fp = fopen(filename, "r");
    if (!fp) {
        return -errno;
    }

    line[0] = '\0';
    section[0] = '\0';

    for (; ; lineno++) {
        int r = read_line(fp, sizeof(buffer), buffer);
        if (r < 0) {
            r = callback(E2BIG, lineno, NULL, NULL, NULL);
            if (r) {
                fclose(fp);
                return r;
            }

            line[0] = '\0';
            continue;

        } else if (r == 0) {
            // EOF.
            return 0;
        }

        p = skip_ws(buffer);
        if (*p == '\0' || is_comment_character(*p)) {
            // Empty, blank, or comment line is ignored.
            continue;
        }

#ifndef NO_BACKSLASH_CONCATENATE
        if (buffer[r - 2] == '\\') {
            // Concatenate Lines ending in backslash. [2]
            buffer[r - 2] = ' ';
            strcat(line, p);
            continue;
        }
#endif

        strcat(line, p);

        p = line;

        if (*p == '[') {
            // Section.
            p++;
            for (; is_section_character(*p); p++) {
            }

            if (!*p) {
                r = callback(ENOTDIR, lineno, NULL, NULL, NULL);

            } else if (*p == ']' && p == line + 1) {
                r = callback(ENOTDIR, lineno, NULL, NULL, NULL);

            } else if (*p != ']' || !is_empty_line(p + 1)) {
                r = callback(ENOTDIR, lineno, NULL, NULL, NULL);

            } else {
                *p = 0;
                strcpy(section, line + 1);
                r = 0;
            }

        } else {
            // Entry key=value.
            for (; is_key_character(*p); p++) {
            }

            // Ignore space before the equals sign. [1]
            if (isspace(*p)) {
                *p++ = 0;
                p = skip_ws(p);
            }

            if (!*p) {
                r = callback(ENOENT, lineno, NULL, NULL, NULL);

            } else if (*p == '=' && p == line) {
                r = callback(ENOENT, lineno, NULL, NULL, NULL);

            } else if (*p != '=') {
                r = callback(ENOENT, lineno, NULL, NULL, NULL);

            } else {
                // Ignore space after the equals sign. [1]
                *p++ = 0;
                r = callback(0, lineno, section, line, skip_ws(p));
            }
        }

        if (r) {
            return r;
        }

        line[0] = '\0';
    }
}
