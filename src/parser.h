#ifndef BFC_PARSER_H
#define BFC_PARSER_H

#include "./bfc_ir.h"
#include "../alib/src/array.h"
#include "../alib/src/vlarray.h"

/*
 * BF GRAMMAR
 * 
 * program  := exprs
 * exprs    := expr | exprs
 * expr     := blocks | keywords
 * blocks   := block | blocks
 * block    := [ keywords ]
 * keywords := keyword | keywords
 * keyword  := > < + - . ,
 */

typedef struct BFSyntaxTree BFSyntaxTree;

/*
 * [ xxxxxxxx [ yyy ] zzz [  uuu ] [ vvvv ] ]
 * xxxxxxxx[yyy]zzz[uuu][vvvv]]
 *
 * */

typedef struct BFCodeLex BFCodeLex;

struct BFCodeLex
{
    char isSection;
    size_t count;
    typedef struct BFCodeSection {
        const BFCodeLex *seq;
    } section;
    typedef struct BFCodeSequence {
        // label
        const BFIR *bfCode;
    } sequence;
};

struct BFSyntaxTree
{
    VLArray *raw;
    // start
};

BFSyntaxTree *parseBF(const Array *BFIR);

#endif



