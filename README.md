# ini
C .ini File Parser

## Example

```c
#include <errno.h>
#include <libini/ini.h>
#include <stdio.h>

static int callback(int e, unsigned lineno, const char *section, const char *key, const char *value)
{
    switch (e) {
        case 0:
            printf("[%s] '%s'='%s'\n", section, key, value);
            break;
        case E2BIG:
            printf("error: line %u, too long\n", lineno);
            break;
        case ENOTDIR:
            printf("error: line %u, bad section\n", lineno);
            break;
        case ENOENT:
            printf("error: line %u, bad entry\n", lineno);
            break;
    }

    return 0;
}

int main(void)
{
    int r = ini_read("data/example.ini", callback);
    if (r < 0) {
        errno = -r;
        perror("example");
    }
}
```

## Installation

```bash
./configure
make
sudo make install
```

## Requirements

- C99 or later

## Thread Safety

This library is thread-safe.
