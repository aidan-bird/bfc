/* aidan bird 2021 */

#include <stdlib.h>
#include <string.h>

#include "./bfc_ir.h"
#include "../alib/src/array.h"
#include "../alib/src/vlarray.h"
#include "../alib/src/utils.h"

static const BFKeyword _charKeywordMap[] = {
    ['>'] = BFKeyword_up,
    ['<'] = BFKeyword_down,
    ['+'] = BFKeyword_inc,
    ['-'] = BFKeyword_dec,
    ['.'] = BFKeyword_print,
    [','] = BFKeyword_read,
    ['['] = BFKeyword_setLabel,
    [']'] = BFKeyword_jump,
};

static const char _keywordCharMap[] = {
    [BFKeyword_up]        = '>',
    [BFKeyword_down]      = '<',
    [BFKeyword_inc]       = '+',
    [BFKeyword_dec]       = '-',
    [BFKeyword_print]     = '.',
    [BFKeyword_read]      = ',',
    [BFKeyword_setLabel]  = '[',
    [BFKeyword_jump]      = ']',
};

static const BFKeyword _keywords[] = {
    BFKeyword_up, BFKeyword_down, BFKeyword_inc, BFKeyword_dec,
    BFKeyword_print, BFKeyword_read, BFKeyword_setLabel, BFKeyword_jump,
    BFKeyword_eof
};

static size_t _maxBfKeyword;
static size_t _bfKeywordCount;

const BFKeyword *charKeywordMap = _charKeywordMap;
const char *keywordCharMap = _keywordCharMap;
const BFKeyword *bfKeywords = _keywords;
const size_t *maxBfKeyword;
const size_t *bfKeywordCount;

void
initBFCIR()
{
    maxBfKeyword = &_maxBfKeyword;
    bfKeywordCount = &_bfKeywordCount;
    _maxBfKeyword = 0;
    for (size_t i = 0; i < LEN(_keywords); i++)
        _maxBfKeyword = MAX(_maxBfKeyword, _keywords[i]);
    _bfKeywordCount = LEN(_keywords);
}

/*
 * returns non-zero if c is a BF keyword
 */
int
isBFCKeyword(int c)
{
    switch (c) {
        case '>':
        case '<':
        case '+':
        case '-':
        case '.':
        case ',':
        case '[':
        case ']':
            return 1;
        default:
            return 0;
    }
}

/*
 * MODIFIES
 * src
 *
 * EFFECTS
 * removes non BF keywords from src inplace.
 */
void
removeNonBFKeywordsInplace(char *src)
{
    char *next;
    
    next = src;
    while (*src) {
        if (isBFCKeyword(*src)) {
            if (src != next)
                *next = *src;
            next++;
        }
        src++;
    }
    *next = '\0';
}

/*
 * EFFECTS
 * counts the number of consecutive reoccurring chars starting at str
 */
size_t
countConsecutiveChars(const char *str)
{
    int c;
    const char *tmp;

    c = *str;
    tmp = str;
    while (*str && *str == c)
        str++;
    return str - tmp;
}

/*
 * REQUIRES
 * src only contains BF keywords
 *
 * EFFECTS
 * creates an array of BFIR from BF src.
 */
Array *
srcToBFIR(const char *src)
{
    Array *ret;
    size_t count;
    BFIR nextBFIR;
    const BFIR termIR = (BFIR) {
        .count = 0,
        .keyword = BFKeyword_eof,
    };

    ret = newArray(-1, -1, sizeof(BFIR));
    if (!ret)
        goto error1;
    while (*src) {
        nextBFIR.keyword = _charKeywordMap[*src];
        if (nextBFIR.keyword == BFKeyword_setLabel
            || nextBFIR.keyword == BFKeyword_jump) {
            count = 1;
        } else {
            count = countConsecutiveChars(src);
        }
        nextBFIR.count = count;
        src += count;
        if (!tryPushArray(&ret, &nextBFIR))
            goto error2;
    }
    if (!tryPushArray(&ret, &termIR))
        goto error2;
    return ret;
error2:;
    deleteArray(ret);
error1:;
    return NULL;
}

/*
 * returns non-zero if keyword is a basic BF keyword.
 */
int
isBasicBFKeyword(BFKeyword keyword)
{
    switch (keyword) {
        case BFKeyword_up:
        case BFKeyword_down:
        case BFKeyword_inc:
        case BFKeyword_dec:
            return 1;
        default:
            return 0;
    }
}

/*
 * returns non-zero if keyword is a IO keyword
 */
int
isIOBFKeyword(BFKeyword keyword)
{
    switch (keyword) {
        case BFKeyword_print:
        case BFKeyword_read:
            return 1;
        default:
            return 0;
    }
}

/*
 * returns non-zero if keyword emits branch instructions
 */
int
isBranchBFKeyword(BFKeyword keyword)
{
    switch (keyword) {
        case BFKeyword_setLabel:
        case BFKeyword_jump:
            return 1;
        default:
            return 0;
    }
}

/*
 * REQUIRES
 * bfir is valid
 *
 * MODIFIES
 * str
 *
 * EFFECTS
 * pushes the keyword associated with the bfir onto str
 * returns non-zero on error.
 */
int
pushBFIRString(VLArray **str, const BFIR *restrict bfir)
{
    char textRep;
    char buf[bfir->count + 1];

    textRep = _keywordCharMap[bfir->keyword];
    memset(buf, textRep, bfir->count);
    buf[bfir->count] = '\0';
    return !tryPushVLArray(str, buf, sizeof(buf));
}


