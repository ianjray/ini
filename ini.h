#ifndef LIBINI_INI_H_
#define LIBINI_INI_H_

/// Callback function.
/// @param err Error code.
///   - 0: Success.
///   - E2BIG: Line too long.
///   - ENOTDIR: Bad section name.
///   - ENOENT: Bad entry name.
/// @param lineno Line number in file.
/// @param section Section name, or NULL if none.
/// @param key Key name,
/// @param value Value name, or NULL if none.
/// @return Zero if reading should continue, non-zero otherwise.
typedef int (*ini_callback_t)(int err, unsigned lineno, const char *section, const char *key, const char *value);

/// Read configuration file.
/// @discussion The configuration file contains zero, one, or more lines.
/// Each line is either a section, an entry, or empty.
/// Sections are of the form @c [NAME].
/// Entries are of the form @c KEY=VALUE where @c VALUE is optional.
/// An empty line is defined as either empty, blank (whitespace only), or starting with a comment character.
/// The comment character is `#' or `;'.
/// Non-empty lines ending in backslash are concatenated with the following line and the backslash is replaced by a single space character.
/// Preprocessor macro NO_BACKSLASH_CONCATENATE is provided to disable this behaviour.
/// @see https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s03.html
/// @see https://www.freedesktop.org/software/systemd/man/systemd.syntax.html
/// @return int Zero on success, return value of @c callback otherwise.
int ini_read(const char *filename, ini_callback_t callback);

#endif // LIBINI_INI_H_
