#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./parser.h"
#include "./bfc_ir.h"
#include "../alib/src/array.h"
#include "../alib/src/vlarray.h"
#include "../alib/src/utils.h"

typedef const BFIR * (*ParsingFunc)(BFSyntaxTree *, const BFIR *,
    Array **section);
int pushBFSection(BFSyntaxTree *syntaxTree, Array **section);
int pushBFCodeSequence(BFSyntaxTree *syntaxTree, const BFIR *bfcode, size_t n,
    LexType lexType, Array **section);
const BFIR *parseIOExpr(BFSyntaxTree *syntaxTree, const BFIR *bfcode,
    Array **section);
const BFIR *parseBasicExpr(BFSyntaxTree *syntaxTree, const BFIR *bfcode,
    Array **section);
const BFIR *parseExpr(BFSyntaxTree *syntaxTree, const BFIR *bfcode,
    Array **section);
const BFIR *parseExprs(BFSyntaxTree *syntaxTree, const BFIR *bfcode,
    Array **section);
const BFIR *parseSection(BFSyntaxTree *syntaxTree, const BFIR *bfcode,
    Array **section);

int
pushBFCodeSequence(BFSyntaxTree *syntaxTree, const BFIR *bfcode, size_t n,
    LexType lexType, Array **section)
{
    size_t bfcodeIndex;
    size_t nextSequenceIndex;

    /* push BF code sequence descriptor */
    if (!tryPushVLArray(&syntaxTree->raw, NULL, sizeof(BFCodeLex)))
        goto error1;
    nextSequenceIndex = lastIndexVLArray(syntaxTree->raw);
    /* push bfcode */
    if (!tryPushVLArray(&syntaxTree->raw, bfcode, sizeof(BFIR) * n))
        goto error1;
    /* setup BF code sequence descriptor */
    bfcodeIndex = lastIndexVLArray(syntaxTree->raw);
    *((BFCodeLex *)getElementVLArray(syntaxTree->raw, nextSequenceIndex))
        = (BFCodeLex) {
        .count = n,
        .isSection = 0,
        .children.sequence.bfCodeIndex = bfcodeIndex,
    };
    if (section) {
        if (!tryPushArray(section, &nextSequenceIndex))
            goto error1;
    }
    return 0;
error1:;
    return 1;
}

int
pushBFSection(BFSyntaxTree *syntaxTree, Array **section)
{
    size_t nextSectionIndex;

    /* push section descriptor */
    if (!tryPushVLArray(&syntaxTree->raw, NULL, sizeof(BFCodeLex)))
        goto error1;
    nextSectionIndex = lastIndexVLArray(syntaxTree->raw);
    /* initialize section descriptor */
    *((BFCodeLex *)getElementVLArray(syntaxTree->raw, nextSectionIndex))
        = (BFCodeLex) {
        .count = 0, .lexType = LexType_Branch,
        .isSection = 1,
        .children.section.seqIndex = 0,
    };
    if (section) {
        if (!tryPushArray(section, &nextSectionIndex))
            goto error1;
    }
    return 0;
error1:;
    return 1;
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
    if (pushBFCodeSequence(syntaxTree, bfcode, i, LexType_IO, section))
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
    if (pushBFCodeSequence(syntaxTree, bfcode, i, LexType_Basic, section))
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
    int thisSectionChildrenIsArray;
    Array *thisSectionChildren;
    size_t thisSectionIndex;

    if (bfcode->keyword != BFKeyword_setLabel)
        return bfcode;
    if (pushBFSection(syntaxTree, section))
        goto error1;
    thisSectionIndex = lastIndexVLArray(syntaxTree->raw);
    thisSectionChildrenIsArray = 1;
    thisSectionChildren = newArray(-1, -1, sizeof(BFCodeLex *));
    if (!thisSectionChildren)
        goto error1;
    ret = parseExprs(syntaxTree, bfcode + 1, &thisSectionChildren);
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
    syntaxTree->nodes++;
    childrenCount = thisSectionChildren->count;
    thisSectionChildrenIsArray = 0;
    arrayToRaw(thisSectionChildren, -1);
    if (!tryPushVLArray(&syntaxTree->raw, thisSectionChildren,
        sizeof(size_t) * childrenCount)) {
        goto error2;
    }
    *((BFCodeLex *)getElementVLArray(syntaxTree->raw, thisSectionIndex))
        = (BFCodeLex) {
        .isSection = 1,
        .count = childrenCount,
        .children.section.seqIndex = lastIndexVLArray(syntaxTree->raw),
    };
    free(thisSectionChildren);
    return ret + 1;
error2:;
    if (thisSectionChildrenIsArray)
        deleteArray(thisSectionChildren);
    else
        free(thisSectionChildren);
error1:;
    return NULL;
}

void
deleteBFSyntaxTree(BFSyntaxTree *tree)
{
    deleteVLArray(tree->raw);
    free(tree);
}

int
printBFSyntaxTreeWhitespace(VLArray **str, size_t depth)
{
    char tmp[depth + 1];

    memset(tmp, '\t', depth);
    tmp[depth] = '\0';
    return !tryPushVLArray(str, tmp, sizeof(tmp));
}

int
printBFSyntaxTreeRecurse(const BFSyntaxTree *tree, const BFCodeLex *node,
    VLArray **str, size_t depth)
{
    const size_t *subNodes;
    const BFCodeLex *subNode;
    const BFIR *ir;

    if (node->isSection) {
        /* RECURSIVE CASE */
        subNodes = (const size_t *)getElementVLArray(tree->raw,
            node->children.section.seqIndex);
        for (size_t i = 0; i < node->count; i++) {
            subNode = (const BFCodeLex *)getElementVLArray(tree->raw,
                subNodes[i]);
            if (printBFSyntaxTreeRecurse(tree, subNode, str, depth + 1))
                return 1;
        }
    } else {
        /* BASE CASE */
        ir = (const BFIR *)getElementVLArray(tree->raw,
            node->children.sequence.bfCodeIndex);
        if (printBFSyntaxTreeWhitespace(str, depth))
            return 1;
        for (size_t i = 0; i < node->count; i++) {
            if (pushBFIRString(str, ir + i))
                return 1;
        }
        static const char *newlineStr = "\n";
        if (!tryPushVLArray(str, newlineStr, 2))
            return 1;
    }
    return 0;
}

char *
printBFSyntaxTree(const BFSyntaxTree *tree)
{
    char *ret;
    VLArray *tmp;

    tmp = newVLArray(-1, -1, -1);
    if (!tmp)
        goto error1;
    if (printBFSyntaxTreeRecurse(tree, tree->start, &tmp, 0))
        goto error2;
    ret = toStringVLArray(tmp);
    if (!ret)
        goto error2;
    deleteVLArray(tmp);
    return ret;
error2:;
    deleteVLArray(tmp);
error1:;
    return NULL;
}

int
parseRoot(BFSyntaxTree *syntaxTree, const BFIR *bfcode, Array **section)
{
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
    ret->nodes = 0;

    // if (!parseSection(ret, (const BFIR *)getElementArray(bfir, 0), NULL))
    //     goto error3;
    if (!parseRoot(ret, (const BFIR *)getElementArray(bfir, 0), NULL))
        goto error3;

    ret->start = (const BFCodeLex *)getElementVLArray(ret->raw, 0);
    return ret;
error3:;
    free(ret);
error2:;
    deleteVLArray(raw);
error1:;
    return NULL;
}

