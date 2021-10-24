#include <stdlib.h>
#include <stdio.h>

#include "./parser.h"
#include "./bfc_ir.h"
#include "../alib/src/array.h"
#include "../alib/src/vlarray.h"
#include "../alib/src/utils.h"

/* XXX store index instead of pointer!!! */

typedef const BFIR * (*ParsingFunc)(BFSyntaxTree *, const BFIR *,
    Array **section);

BFCodeLex *
pushEmptyBFCodeSequence(BFSyntaxTree *syntaxTree)
{
    BFCodeLex *ret;

    /* push BF code sequence descriptor */
    if (!tryPushVLArray(&syntaxTree->raw, NULL, sizeof(BFCodeLex)))
        goto error1;
    ret = (BFCodeLex *)peekVLArray(syntaxTree->raw);
    /* setup BF code sequence descriptor */
    *ret = (BFCodeLex) {
        .count = 0,
        .lexType = LexType_Empty,
        .isSection = 0,
        .sequence.bfCodeIndex = 0,
    };
    return ret;
error1:;
    return NULL;
}

BFCodeLex *
pushBFCodeSequence(BFSyntaxTree *syntaxTree, const BFIR *bfcode, size_t n,
    LexType lexType)
{
    BFCodeLex *nextSequence;

    /* push BF code sequence descriptor */
    if (!tryPushVLArray(&syntaxTree->raw, NULL, sizeof(BFCodeLex)))
        goto error1;
    nextSequence = (BFCodeLex *)peekVLArray(syntaxTree->raw);
    /* push bfcode */
    if (!tryPushVLArray(&syntaxTree->raw, bfcode, sizeof(BFIR) * n))
        goto error1;
    /* setup BF code sequence descriptor */
    *nextSequence = (BFCodeLex) {
        .count = n,
        .isSection = 0,
        .sequence.bfCode = (const BFIR *)peekVLArray(syntaxTree->raw),
    };
    return nextSequence;
error1:;
    return NULL;
}

const BFIR *
parseIOExpr(BFSyntaxTree *syntaxTree, const BFIR *bfcode, Array **section)
{
    size_t i;

    i = 0;
    while (bfcode[i].keyword != BFKeyword_eof) {
        if (!isIOBFKeyword(bfcode[i].keyword))
            break;
        i++;
    }
    if (!i) {
        /* no IO exprs */
        return bfcode;
    }
    if (!pushBFCodeSequence(syntaxTree, bfcode, i, LexType_IO))
        goto error1;
    if (!tryPushArray(section, (BFCodeLex *)peekVLArray(syntaxTree->raw)))
        goto error1;
    return bfcode + i;
error1:;
    return NULL;
}

const BFIR *
parseBasicExpr(BFSyntaxTree *syntaxTree, const BFIR *bfcode, Array **section)
{
    size_t i;

    i = 0;
    while (bfcode[i].keyword != BFKeyword_eof) {
        if (!isBasicBFKeyword(bfcode[i].keyword))
            break;
        i++;
    }
    if (!i) {
        /* no basic exprs */
        return bfcode;
    }
    if (!pushBFCodeSequence(syntaxTree, bfcode, i, LexType_Basic))
        goto error1;
    if (!tryPushArray(section, (BFCodeLex *)peekVLArray(syntaxTree->raw)))
        goto error1;
    return bfcode + i;
error1:;
    return NULL;
}

/* expr := IOExpr || BasicExpr */
const BFIR *
parseExpr(BFSyntaxTree *syntaxTree, const BFIR *bfcode, Array **section)
{
    const BFIR *ret;
    const ParsingFunc funcs[] = { parseBasicExpr, parseIOExpr, parseSection };

    for (size_t i = 0; i < LEN(funcs); i++) {
        ret = funcs[i](syntaxTree, bfcode, section);
        if (!ret)
            return NULL;
        if (ret != bfcode)
            break;
    }
    return ret;
}

/* exprs = expr || expr exprs */
const BFIR *
parseExprs(BFSyntaxTree *syntaxTree, const BFIR *bfcode, Array **section)
{
    const BFIR *ret;

    ret = parseExpr(syntaxTree, bfcode, section);
    if (!ret)
        return NULL;
    if (ret == bfcode)
        return ret;
    return parseExprs(syntaxTree, ret, section);
}

/* section := [ exprs ] */
const BFIR *
parseSection(BFSyntaxTree *syntaxTree, const BFIR *bfcode, Array **section)
{
    const BFIR *ret;
    size_t childrenCount;
    int isNewSubSectionArray;
    BFCodeLex *thisSection;
    Array *newSubSection;

    isNewSubSectionArray = 1;
    if (bfcode->keyword != BFKeyword_setLabel)
        return bfcode;
    if (!tryPushVLArray(&syntaxTree->raw, NULL, sizeof(BFCodeLex)))
        goto error1;
    thisSection = (BFCodeLex *)peekVLArray(syntaxTree->raw);
    newSubSection = newArray(-1, -1, sizeof(BFCodeLex *));
    if (!newSubSection)
        goto error1;
    ret = parseExprs(syntaxTree, bfcode + 1, &newSubSection);
    if (!ret)
        goto error2;
    /* detect empty brackets [] */
    // if (ret == bfcode + 1)
    //     return bfcode;
    if (ret->keyword != BFKeyword_jump) {
        fputs("Warning: unmatched [ bracket", stderr);
        goto error2;
        // return ret;
    }
    /* brackets are closed forming a section */
    childrenCount = newSubSection->count;
    isNewSubSectionArray = 0;
    arrayToRaw(newSubSection, -1);
    if (!tryPushVLArray(&syntaxTree->raw, newSubSection,
        sizeof(BFCodeLex) * childrenCount)) {
        goto error2;
    }
    *thisSection = (BFCodeLex) {
        .isSection = 1,
        .count = childrenCount,
        .section.seq = (const BFCodeLex *)peekVLArray(syntaxTree->raw),
    };
    free(newSubSection);
    return ret + 1;
error2:;
    if (isNewSubSectionArray)
        deleteArray(newSubSection);
    else
        free(newSubSection);
error1:;
    return NULL;
}

void
deleteBFSyntaxTree(BFSyntaxTree *tree)
{
    deleteVLArray(tree->raw);
    free(tree);
}

BFSyntaxTree *
parseBF(const Array *bfir)
{
    BFSyntaxTree *ret;
    VLArray *raw;
    const BFCodeLex *start;

    raw = newVLArray(-1, -1, sizeof(void *));
    if (!raw)
        goto error1;
    ret = malloc(sizeof(BFSyntaxTree));
    if (!ret)
        goto error2;
    ret->raw = raw;
    if (!parseSection(ret, (const BFIR *)getElementArray(bfir, 0), NULL))
        goto error3;
    ret->start = (const BFCodeLex *)getElementVLArray(ret->raw, 0);
//    start = pushBFCodeSection(ret, (const BFIR *)getElementArray(bfir, 0),
//        bfir->count, 0, NULL);
    // if (!start)
    //     goto error3;
    // ret->start = start;
    return ret;
error3:;
    free(ret);
error2:;
    deleteVLArray(raw);
error1:;
    return NULL;
}

