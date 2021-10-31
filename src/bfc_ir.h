#ifndef BFC_IR_H
#define BFC_IR_H

#include <stddef.h>

#include "../alib/src/array.h"
#include "../alib/src/vlarray.h"

typedef struct BFIR BFIR;

enum BFKeyword
{
    BFKeyword_up,
    BFKeyword_down,
    BFKeyword_inc,
    BFKeyword_dec,
    BFKeyword_print,
    BFKeyword_read,
    BFKeyword_setLabel,
    BFKeyword_jump,
    BFKeyword_eof,
};

typedef enum BFKeyword BFKeyword;

struct BFIR
{
    size_t count;
    BFKeyword keyword;
};

Array *srcToBFIR(const char *src);
int isBFCKeyword(int c);
void removeNonBFKeywordsInplace(char *src);
int isBasicBFKeyword(BFKeyword keyword);
int isIOBFKeyword(BFKeyword keyword);
int isBranchBFKeyword(BFKeyword keyword);
int pushBFIRString(VLArray **str, const BFIR *restrict bfir);

extern const BFKeyword *charKeywordMap;
extern const char *keywordCharMap;

#endif
