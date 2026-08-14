#include "weft/weft.h"

#include <string.h>

int main(int argc, char const* argv[])
{
  (void)argc;
  (void)argv;

  return strcmp(exported_function(), "weft") == 0 ? 0 : 1;
}
