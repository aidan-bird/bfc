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
    }
    return 0;
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
    size_t ret;
    int c;

    ret = 0;
    c = *str;
    while (*str && *str == c)
        str++;
    return ret;
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
    Array *tmp;
    size_t count;
    BFIR nextBFIR;

    ret = newArray(-1, -1, sizeof(BFIR));
    if (!ret)
        goto error1;
    while (*src) {
        nextBFIR.keyword = _charKeywordMap[*src];
        count = countConsecutiveChars(src);
        nextBFIR.count = count;
        src += count;
        tmp = pushArray(ret, &nextBFIR);
        if (!tmp)
            goto error2;
        ret = tmp;
        src++;
    }
    return ret;
error2:;
    deleteArray(ret);
error1:;
    return NULL;
}

