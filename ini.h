#ifndef INI__H
#define INI__H

/// Read configuration file.
/// @discussion The configuration file contains zero, one, or more lines.
/// Each line is either a section, an entry, or empty.
/// An empty line is defined as either empty, blank (whitespace only), or starting with a comment character.
/// The comment character is `#' or `;'.
/// Non-empty lines ending in backslash are concatenated with the following line and the backslash is replaced by a single space character.
/// Preprocessor macro NO_BACKSLASH_CONCATENATE is provided to disable this behaviour.
/// @see https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s03.html
/// @see https://www.freedesktop.org/software/systemd/man/systemd.syntax.html
/// @return int Zero on success, negative errno otherwise.
int ini_read(const char *filename, void (*callback)(const char *section, const char *key, const char *value));

#endif
