# ini
C .ini File Parser

## Example

```
static void callback(const char * section, const char * key, const char * value)
{
    printf("[%s] '%s'='%s'\n", section, key, value);
}

int r = ini_read(filename, callback);
if (r < 0) {
    errno = -r;
    perror(filename);
}
```
