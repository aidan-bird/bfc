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
    char *asmCode;
    char *src;
    Array *bfir;
    BFSyntaxTree *tree;
    Translator *trans;

    initBFCIR();
    if (!(src = readTextFile(stdin, NULL)))
        goto error1;
    removeNonBFKeywordsInplace(src);
    if (!(bfir = srcToBFIR(src)))
        goto error2;
    if (!(tree = parseBF(bfir)))
        goto error3;
    if (!(trans = newTranslator(ISA_x86_64, Platform_linux, tree)))
        goto error4;
    if (translate(trans))
        goto error5;
    if (!(asmCode = translateToString(trans, NULL)))
        goto error6;
    puts(asmCode);
    free(src);
    deleteArray(bfir);
    deleteBFSyntaxTree(tree);
    deleteTranslator(trans);
    free(asmCode);
    return 0;
error6:
error5:
    deleteTranslator(trans);
error4:
    deleteBFSyntaxTree(tree);
error3:
    deleteArray(bfir);
error2:
    free(src);
error1:
    fputs("Errors occurred during compilation.", stderr);
    return 1;
}
