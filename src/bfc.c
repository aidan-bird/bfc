/* aidan bird 2021 */

#include <stdio.h>
#include <stdlib.h>

#include "./bfc_ir.h"
#include "./parser.h"
#include "./translate.h"
#include "../alib/src/utils.h"
#include "../alib/src/array.h"

#include "../alib/src/string_builder.h"

int
main(int argc, char **argv)
{
    char *str;
    char *asmCode;
    char *src;
    Array *bfir;
    BFSyntaxTree *tree;
    Translator *trans;

    initBFCIR();
    src = readTextFile(stdin, NULL);
    removeNonBFKeywordsInplace(src);
    bfir = srcToBFIR(src);
    tree = parseBF(bfir);
    str = printBFSyntaxTree(tree);
    puts(str);
    trans = newTranslator(ISA_x86_64, Platform_linux, tree);
    translate(trans);
    asmCode = translateToString(trans, NULL);
    if (asmCode)
        puts(asmCode);
    if (str)
        free(str);
    deleteBFSyntaxTree(tree);
    free((char *)src);
    deleteArray(bfir);
    return 0;
}
