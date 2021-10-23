#include "./parser.h"
#include "../alib/src/array.h"
#include "../alib/src/vlarray.h"

BFCodeLex *
pushBFCodeSequence(BFSyntaxTree *syntaxTree, const BFIR *bfcode, size_t n)
{
    VLArray *tmp;
    BFCodeLex nextSequence;
    BFCodeLex *nextSequenceAddr;

    /* setup BF code sequence descriptor */
    nextSequence.count = n;
    nextSequence.isSection = 0;
    /* push BF code sequence descriptor */
    tmp = pushVLArray(syntaxTree->raw, &nextSequence, sizeof(nextSequence));
    if (!tmp)
        goto error1;
    syntaxTree->raw = tmp;
    /* push bfcode */
    tmp = pushVLArray(syntaxTree->raw, bfcode, sizeof(BFIR) * n);
    if (!tmp)
        goto error1;
    syntaxTree->raw = tmp;
    /* further setup BF code sequence descriptor */
    nextSequenceAddr = (BFCodeLex *)getElementVLArray(syntaxTree->raw, 
        getCountVLArray(syntaxTree->raw) - 2);
    nextSequenceAddr->sequence.bfCode 
        = (const BFIR *)peekVLArray(syntaxTree->raw);
    return nextSequenceAddr;
error1:;
    return NULL;
}

BFCodeLex *
pushBFCodeSection(BFSyntaxTree *syntaxTree, const BFIR *bfcode, size_t n,
    int exitOnJump)
{
    Array *sequenceStack;
    Array *tmp;
    BFCodeLex nextSection;
    BFCodeLex *nextSequenceAddr;
    BFCodeLex *thisCodeSection;

    sequenceStack = newArray(-1, -1, sizeof(BFCodeLex *));
    if (!sequenceStack)
        goto error1;
    nextSequence.isSection = 1;
    tmp = pushVLArray(syntaxTree->raw, &nextSequence, sizeof(nextSequence));
    if (!tmp)
        goto error2;
    thisCodeSection = (BFCodeLex *)peekVLArray(syntaxTree->raw);
    syntaxTree->raw = tmp;
    // nextSequence.label = xyz;
    for (size_t i = 0; i < n 
        || (exitOnJump && (bfcode[i].keyword == BFKeyword_jump)); i++) {
        /* find sub-section */
        for (size_t j = i; j < n 
            || (exitOnJump && (bfcode[j].keyword == BFKeyword_jump)); j++) {
            if (bfcode[j].keyword == BFKeyword_setLabel) {
                /* make new code sequence that spans from i to j */
                nextSequenceAddr = pushBFCodeSequence(syntaxTree, bfcode + i,
                    j - i);
                if (!nextSequenceAddr)
                    goto error2;
                tmp = pushArray(sequenceStack, nextSequenceAddr);
                if (!tmp)
                    goto error2;
                sequenceStack = tmp;
                /* recurse into the subsection that comes after j */
                nextSequenceAddr = pushBFCodeSection(syntaxTree,
                    bfcode + j + 1, n - j - 1, 1);
                if (!nextSequenceAddr)
                    goto error2;
            }
        }
    }

    for (size_t i = 0; i < sequenceStack->count; i++) {
        (BFCodeLex *)getElementArray(sequenceStack, i);
    }

    //tmp = pushVLArray(syntaxTree->raw, &nextSequence, sizeof(nextSequence));
    //if (!tmp)
    //    goto error1;
    //syntaxTree->raw = tmp;

    }
    // if (exitOnJump) {
    // }






    deleteArray(sequenceStack);
    return ret;
error2:;
    deleteArray(sequenceStack);
error1:;
    return NULL;
}

BFSyntaxTree *
parseBF(const Array *BFIR)
{
    BFSyntaxTree *ret;
    VLArray *raw;

    raw = newVLArray(-1, -1, sizeof(BFCodeSection));
    if (!raw)
        goto error1;




    return ret;
error1:;
    return NULL;
}

