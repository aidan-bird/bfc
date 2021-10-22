/* aidan bird 2021 */

#include <stdio.h>
#include <stdlib.h>

#include "../alib/src/utils.h"
#include "../alib/src/array.h"

int
main(int argc, char **argv)
{
    const char *src;

    src = readTextFile(stdin, NULL);
    puts(src);
    free((char *)src);
    return 0;
}
