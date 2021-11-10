#ifndef BFC_TRANSLATE_H
#define BFC_TRANSLATE_H

#include "./parser.h"
#include "./bfc_ir.h"

#include "../alib/src/string_builder.h"

enum ISA
{
    ISA_x86_64,
    ISA_ia32,
};

enum PlatformName
{
    Platform_linux,
};

typedef enum ISA ISA;
typedef enum PlatformName PlatformName;
typedef struct Translator Translator;
typedef int (*CodeEmitterFunc)(Translator *translator, const BFIR *bfir);
typedef struct PlatformSpec PlatformSpec;

struct PlatformSpec
{
    const char *name;
    CodeEmitterFunc emitBasic;
    CodeEmitterFunc emitIO;
    CodeEmitterFunc emitSection;
    CodeEmitterFunc emitJump;
    CodeEmitterFunc emitExit;
    CodeEmitterFunc emitStart;
};

struct Translator
{
    size_t nodeIndex;
    PlatformSpec platform;
    StringBuilder *codeBuf;
    const BFSyntaxTree *syntaxTree;
};

Translator * newTranslator(ISA isa, PlatformName platform,
    const BFSyntaxTree *syntaxTree);

char *
translate(Translator *translator);

#endif
