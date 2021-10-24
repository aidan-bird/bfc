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
 * xxx[yyy]zzz[]
 * n = 19
 * dd = 7
 * ii = 12
 * i = 8
 * j = 10
 *
 * yyyx
 * i = 3
 * j = 12
 * n = 10
 *
 * */

typedef struct BFCodeLex BFCodeLex;

enum LexType {
    LexType_Basic,
    LexType_IO,
    LexType_Branch,
    LexType_Empty,
};

typedef enum LexType LexType;

struct BFCodeLex
{
    char isSection;
    LexType lexType;
    size_t count;
    union {
        struct BFCodeSection {
            size_t seqIndex;
        } section;
        struct BFCodeSequence {
            size_t bfCodeIndex;
        } sequence;
    };
};

struct BFSyntaxTree
{
    VLArray *raw;
    const BFCodeLex *start;
};

BFSyntaxTree *parseBF(const Array *BFIR);
void deleteBFSyntaxTree(BFSyntaxTree *tree);
BFCodeLex *pushEmptyBFCodeSequence(BFSyntaxTree *syntaxTree);
BFCodeLex *pushBFCodeSequence(BFSyntaxTree *syntaxTree, const BFIR *bfcode,
    size_t n, LexType lexType);
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


#endif



