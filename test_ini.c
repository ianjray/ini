#include "ini.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static int ret_;

static int callback(int err, unsigned lineno, const char *section, const char *key, const char *value)
{
    switch (err) {
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

    return ret_;
}

static int test(const char *s, ini_callback_t callback)
{
    const char *filename = "/tmp/conf.ini";
    FILE *fp = fopen(filename, "w");
    assert(fp);
    fwrite(s, strlen(s), 1, fp);
    fclose(fp);
    return ini_read(filename, callback);
}

int main(void)
{
    int r = ini_read("missing", callback);
    assert(r == -ENOENT);

    test(
        "\n"
        "	"
        "	\n"
        " 	#\n#"
        " 	;\n;"
        , callback);

    ret_ = 1;
    test(
        "a\n"
        "=b\n"
        , callback);

    ret_ = 0;
    test(
        "a\n"
        "=b\n"
        , callback);

    test(
        "[]\n"
        "[[\n"
        "[Group names may contain all ASCII characters except for [ and ] & ctrl chars.\n"
        "[Missing section terminator\n"
        "[Trailing stuff]?\n"
        , callback);

    test(
        "[Missing key separator '=']\n"
        "something\n"
        "[Key names may contain characters A-Za-z0-9-.]\n"
        "$huh=$bar\n"
        "[Trailing commented] #comment\n"
        "42=0x2a\n"
        "[Trailing] 	\n"
        "k=v"
        , callback);

    test(
        "[Empty key]\n"
        "=value"
        , callback);

    printf("test long (fail)\n");

    ret_ = 1;
    r = test(
        "[Too long #1]\n"
        "123=567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "12345678901234567890123\n"
        "Long=unrecovered\n"
        , callback);
    assert(r == 1);

    printf("test long (continue)\n");

    ret_ = 0;
    r = test(
        "[Too long #2]\n"
        "123=567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "12345678901234567890123\n"
        "Long=recovered\n"
        , callback);
    assert(r == 0);

    test(
        "[Empty Section]"
        , callback);

    test(
        "[Section]\n"
        "key	 =	\\\n"
        "long\\\n"
        "value\n"
        "  [ Section 2 ]\n"
        "  key  =  value1  \n"
        "key=value2  \n"
        "key=value3\n"
        , callback);

    test(
        "[Section C]\r"
        "Key=Value\\\r"
        "# this line is ignored\r"
        "; this line is ignored too\r"
        "...continued\r"
        "syntax\r"
        , callback);
}
