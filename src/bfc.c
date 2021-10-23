/* aidan bird 2021 */

#include <stdio.h>
#include <stdlib.h>

#include "./bfc_ir.h"
#include "../alib/src/utils.h"
#include "../alib/src/array.h"

int
main(int argc, char **argv)
{
    char *src;
    Array *bfir;

    src = readTextFile(stdin, NULL);
    removeNonBFKeywordsInplace(src);
    bfir = srcToBFIR(src);
    free((char *)src);
    deleteArray(bfir);
    return 0;
}
