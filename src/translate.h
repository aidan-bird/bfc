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
typedef int (*SectionEmitterFunc)(Translator *translator, int sectionID);
typedef struct PlatformSpec PlatformSpec;
typedef struct CodeEmitterConfig CodeEmitterConfig;

struct PlatformSpec
{
    const char *name;
    CodeEmitterFunc emitBasic;
    CodeEmitterFunc emitIO;
    SectionEmitterFunc emitSection;
    CodeEmitterFunc emitExit;
    CodeEmitterFunc emitStart;
    SectionEmitterFunc emitJump;
};

struct Translator
{
    int isTranslated;
    size_t nodeIndex;
    StringBuilder *codeBuf;
    const BFSyntaxTree *syntaxTree;
    const PlatformSpec* spec;
};

Translator * newTranslator(ISA isa, PlatformName platform,
    const BFSyntaxTree *syntaxTree);

char *translateToString(Translator *translator, size_t *outLen);

int translate(Translator *translator);

#endif
