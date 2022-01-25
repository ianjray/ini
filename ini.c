#include "ini.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


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

/// @brief Determine if string contains an empty line.
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

/// @brief Accept character.
/// @discussion Consume @c ch if available.
static void accept_char(FILE *fp, int ch)
{
    int c = fgetc(fp);

    if (c == EOF) {
        return;

    } else if (c != ch) {
        ungetc(c, fp);
    }
}

/// @brief Accept EOL characters.
/// @discussion Accept EOF, CR, LF, CRLF, LFCR as meaning end-of-line.
/// @return bool True if EOL reached.
static bool accept_eol(FILE *fp, int c)
{
    if (c == EOF) {
        return true;

    } else if (c == '\r') {
        // Consume CR[,LF].
        accept_char(fp, '\n');
        return true;

    } else if (c == '\n') {
        // Consume LF[,CR].
        accept_char(fp, '\r');
        return true;
    }

    return false;
}

/// @brief Read line from file.
/// @discussion Read up to EOL or EOF.
/// @return int -ENOSPC if buffer too small, zero on EOF, or positive number of characters written to @c buf (including NUL terminator).
static int read_line(FILE *fp, size_t cap, char *buf)
{
    int len = 0;

    while (cap > 1) {
        int c = fgetc(fp);

        if (c == EOF && len == 0) {
            return 0;

        } else if (accept_eol(fp, c)) {
            *buf = 0;
            len++;
            return len;

        } else {
            *buf++ = (char)c;
            len++;
            cap--;
        }
    }

    return -ENOSPC;
}

int ini_read(const char * filename, void (*callback)(const char * section, const char * key, const char * value))
{
    FILE *fp;
    int lineno = 1;
    char buffer[81];
    char line[80];
    char section[80];
    char *p;

    fp = fopen(filename, "r");
    if (!fp) {
        return -errno;
    }

    line[0] = '\0';

    for (; ; lineno++) {
        int r = read_line(fp, sizeof(buffer), buffer);
        if (r < 0) {
            printf("error, line %d, Line too long.\n", lineno);
            fclose(fp);
            return r;

        } else if (r == 0) {
            // EOF.
            return 0;

        } else {
            p = skip_ws(buffer);
            if (*p == '\0' || is_comment_character(*p)) {
                // Empty, blank, or comment line is ignored.
                continue;
            }

            if (buffer[r - 2] == '\\') {
                // Concatenate Lines ending in backslash. [2]
                buffer[r - 2] = ' ';
                strcat(line, p);
                continue;
            }

            strcat(line, p);
        }

        p = line;

        if (*p == '[') {
            // Section.
            p++;
            for (; is_section_character(*p); p++) {
            }

            if (!*p) {
                printf("error: line %d, Missing section terminator ']'\n", lineno);

            } else if (*p == ']' && p == line + 1) {
                printf("error: line %d, Empty section.\n", lineno);

            } else if (*p != ']' || !is_empty_line(p + 1)) {
                printf("error: line %d, Group names may contain all ASCII characters except for [ and ] and control characters.\n", lineno);

            } else {
                *p = 0;
                strcpy(section, line + 1);
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
                printf("error: line %d, Missing key separator '='.\n", lineno);

            } else if (*p == '=' && p == line) {
                printf("error: line %d, Empty key.\n", lineno);

            } else if (*p != '=') {
                printf("error: line %d, Key names may contain characters A-Za-z0-9-.\n", lineno);

            } else {
                // Ignore space after the equals sign. [1]
                *p++ = 0;
                callback(section, line, skip_ws(p));
            }
        }

        line[0] = '\0';
    }
}
