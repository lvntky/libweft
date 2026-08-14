#include <string.h>

#include "weft/weft.h"

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    return strcmp(exported_function(), "weft") == 0 ? 0 : 1;
}
