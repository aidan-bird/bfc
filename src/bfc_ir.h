#ifndef BFC_IR_H
#define BFC_IR_H

/* aidan bird 2021 */

#include <stddef.h>

#include "../alib/src/array.h"
#include "../alib/src/vlarray.h"

#define isBF_EOF(BFIR_PTR) ((BFIR_PTR)->keyword == BFKeyword_eof)

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
void initBFCIR();

extern const BFKeyword *charKeywordMap;
extern const char *keywordCharMap;
extern const BFKeyword *bfKeywords;
extern const size_t *maxBfKeyword;
extern const size_t *bfKeywordCount;

#endif
