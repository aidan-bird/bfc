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


    StringBuilder *sb;
    sb = newStringBuilder();
    char c = 'a';
    size_t len = 0;
    stringBuilderPushStr(sb, "foobar");
    stringBuilderPushChar(sb, c);
    stringBuilderPrintInplace(sb);
    stringBuilderPushStr(sb, "baz");
    char *fff = stringBuilderToString(sb, &len);
    puts(fff);
    deleteStringBuilder(sb);



    src = readTextFile(stdin, NULL);
    removeNonBFKeywordsInplace(src);
    bfir = srcToBFIR(src);
    tree = parseBF(bfir);
    trans = newTranslator(ISA_ia32, Platform_linux, tree);
    asmCode = translate(trans);

    if (asmCode)
        puts(asmCode);

    // str = printBFSyntaxTree(tree);
    // puts(str);

    if (str)
        free(str);
    deleteBFSyntaxTree(tree);
    free((char *)src);
    deleteArray(bfir);
    return 0;
}
