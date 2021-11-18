#ifndef BFC_PARSER_H
#define BFC_PARSER_H

#include "./bfc_ir.h"
#include "../alib/src/array.h"
#include "../alib/src/vlarray.h"

#define ROOT_SECTION_ID 0
#define START_SECTION_ID 0

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
    LexType_Section,
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
            int sectionID;
            size_t seqIndex;
            size_t endIndex;
        } section;
        struct BFCodeSequence {
            size_t bfCodeIndex;
        } sequence;
        struct BFCodeBranch {
            int sectionID;
            size_t sectIndex;
        } branch;
    } children;
};

struct BFSyntaxTree
{
    size_t nodes;
    int nextSectionID;
    VLArray *raw;
    const BFCodeLex *start;
};

BFSyntaxTree *parseBF(const Array *BFIR);
void deleteBFSyntaxTree(BFSyntaxTree *tree);
char *printBFSyntaxTree(const BFSyntaxTree *tree);
void deleteBFSyntaxTree(BFSyntaxTree *tree);

#endif



