#include "ini.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>


static void callback(const char * section, const char * key, const char * value)
{
    printf("[%s] '%s'='%s'\n", section, key, value);
}

static int test(const char * s, void (*callback)(const char * section, const char * key, const char * value))
{
    const char *filename = "/tmp/conf.ini";
    FILE *fp = fopen(filename, "w");
    assert(fp);
    fwrite(s, strlen(s), 1, fp);
    fclose(fp);
    return ini_read(filename, callback);
}

int main()
{
    int r = ini_read("missing", callback);
    printf("%d\n", r);

    test(
        "\n"
        "	"
        "	\n"
        " 	#\n#"
        " 	;\n;"
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

    test(
        "[Too long #1]\n"
        "123=5678901234567890123456789012345678901234567890123456789012345678901234567890\n"
        , callback);

    test(
        "[Too long #2]\n"
        "123=56789012345678901234567890123456789012345678901234567890123456789012345678901\n"
        , callback);

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
