#include <stdlib.h>

#include "./bfc_ir.h"
#include "../alib/src/array.h"

static const BFKeyword _charKeywordMap[] =
{
    ['>'] = BFKeyword_up,
    ['<'] = BFKeyword_down,
    ['+'] = BFKeyword_inc,
    ['-'] = BFKeyword_dec,
    ['.'] = BFKeyword_print,
    [','] = BFKeyword_read,
    ['['] = BFKeyword_setLabel,
    [']'] = BFKeyword_jump,
};

const BFKeyword *charKeywordMap = _charKeywordMap;

/*
 * REQUIRES
 * none
 *
 * MODIFIES
 * none
 *
 * EFFECTS
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
 * REQUIRES
 * none
 *
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
 * REQUIRES
 * none
 *
 * MODIFIES
 * none
 *
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
 * MODIFIES
 * none
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
