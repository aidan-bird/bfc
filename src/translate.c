#include <stdlib.h>
#include <stdio.h>

#include "./translate.h"

#include "../alib/src/vlarray.h"

static int linux_x86_64_emitBasic(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitIO(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitSection(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitExit(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitStart(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitJump(Translator *translator, const BFIR *bfir);
static int notImplementedYet();

static int
__translateRecursive(Translator *translator, const BFCodeLex *node,
    size_t depth);

static const PlatformSpec linuxPlatformTab[] = {
    [ISA_x86_64] = {
        .name = "linux_x86_64",
        .emitBasic = linux_x86_64_emitBasic,
        .emitIO = linux_x86_64_emitIO,
        .emitSection = linux_x86_64_emitSection,
        .emitExit = linux_x86_64_emitExit,
        .emitStart = linux_x86_64_emitStart,
        .emitJump = linux_x86_64_emitJump,
    },
    /* XXX not implemented yet */
    [ISA_ia32] = {
        .name = "linux_ia32",
        .emitBasic = notImplementedYet,
        .emitIO = notImplementedYet,
        .emitSection = notImplementedYet,
        .emitExit = notImplementedYet,
        .emitStart = notImplementedYet,
        .emitJump = notImplementedYet,
    },
};

static int
notImplementedYet()
{
    puts("Not Implemented Yet!!");
    exit(1);
    return 0;
}

static const PlatformSpec *platformTab[] = {
    [Platform_linux] = linuxPlatformTab,
};

Translator *
newTranslator(ISA isa, PlatformName platform, const BFSyntaxTree *syntaxTree)
{
    Translator *ret;

    ret = malloc(sizeof(Translator));
    if (!ret)
        goto error1;
    ret->codeBuf = newStringBuilder();
    if (!ret->codeBuf)
        goto error2;
    ret->syntaxTree = syntaxTree;
    ret->platform = platformTab[platform][isa];
    ret->nodeIndex = 0;
    return ret;
error2:;
    free(ret);
error1:;
    return NULL;
}

char *
translate(Translator *translator)
{
    translator->platform.emitStart(translator, NULL);
    __translateRecursive(translator, translator->syntaxTree->start, 0);
    translator->platform.emitExit(translator, NULL);
    return NULL;
}

static int
__translateRecursive(Translator *translator, const BFCodeLex *node,
    size_t depth)
{
    const size_t *subNodes;
    const BFCodeLex *subNode;
    const BFIR *ir;

    if (node->isSection) {
        /* RECURSIVE CASE */
        subNodes = (const size_t *)getElementVLArray(
            translator->syntaxTree->raw, node->children.section.seqIndex);
        for (size_t i = 0; i < node->count; i++) {
            subNode = (const BFCodeLex *)getElementVLArray(
                translator->syntaxTree->raw, subNodes[i]);
            if (__translateRecursive(translator, subNode, depth + 1))
                return 1;
        }
    } else {
        /* BASE CASE */
        ir = (const BFIR *)getElementVLArray(
            translator->syntaxTree->raw, node->children.sequence.bfCodeIndex);
        for (size_t i = 0; i < node->count; i++) {
            if (isBasicBFKeyword(ir[i].keyword)) {
                translator->platform.emitBasic(translator, ir + i);
            } else if (isIOBFKeyword(ir[i].keyword)) {
                translator->platform.emitIO(translator, ir + i);
            } else {
                return 1;
            }
        }
        /* emit jump */
        translator->platform.emitJump(translator, NULL);
    }
}

static int
linux_x86_64_emitBasic(Translator *translator, const BFIR *bfir)
{
    switch (bfir->keyword) {
        default:
            return 1;
    }
    return 0;
}

static int
linux_x86_64_emitIO(Translator *translator, const BFIR *bfir)
{
}

static int
linux_x86_64_emitSection(Translator *translator, const BFIR *bfir)
{
}

static int
linux_x86_64_emitExit(Translator *translator, const BFIR *bfir)
{
    static const char exitCode[] = 
"end:\n"
"    mov eax, 1\n"
"    int 0x80";

    return stringBuilderPushStr(translator->codeBuf, exitCode);
}

static int
linux_x86_64_emitStart(Translator *translator, const BFIR *bfir)
{
    static const char *startCode = 
"section .data\n"
"section .text\n"
"    global _start\n"
"_start:\n"
"    push ebp\n"
"    mov ebp, esp\n";

    return stringBuilderPushStr(translator->codeBuf, startCode);
}

static int
linux_x86_64_emitJump(Translator *translator, const BFIR *bfir)
{
    return 0;
}
