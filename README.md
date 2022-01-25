# ini
C .ini File Parser

## Example

```c
#include "ini.h"

static void callback(const char *section, const char *key, const char *value)
{
    printf("[%s] '%s'='%s'\n", section, key, value);
}

int main(void)
{
    int r = ini_read(filename, callback);
    if (r < 0) {
        errno = -r;
        perror(filename);
    }
}
```
