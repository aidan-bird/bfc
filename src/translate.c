/* aidan bird 2021 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "./config.h"
#include "./translate.h"
#include "./bfc_ir.h"
#include "../alib/src/vlarray.h"
#include "../alib/src/utils.h"
#include "./translate_linux_x86_64.h"
#include "./translate_linux_ia32.h"

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
    [ISA_ia32] = {
        .name = "linux_ia32",
        .emitBasic = linux_ia32_emitBasic,
        .emitIO = linux_ia32_emitIO,
        .emitSection = linux_ia32_emitSection,
        .emitExit = linux_ia32_emitExit,
        .emitStart = linux_ia32_emitStart,
        .emitJump = linux_ia32_emitJump,
    },
};

static const PlatformSpec *platformTabs[] = {
    [Platform_linux] = linuxPlatformTab,
};

void
deleteTranslator(Translator *translator)
{
    deleteStringBuilder(translator->codeBuf);
}

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
    ret->nodeIndex = 0;
    ret->spec = platformTabs[platform] + isa;
    ret->isTranslated = 0;
    return ret;
error2:;
    free(ret);
error1:;
    return NULL;
}

int
translate(Translator *translator)
{
    translator->isTranslated = !(translator->spec->emitStart(translator, NULL)
        || __translateRecursive(translator, translator->syntaxTree->start, 0)
        || translator->spec->emitExit(translator, NULL));
    return !translator->isTranslated;
}

char *
translateToString(Translator *translator, size_t *outLen)
{
    return !translator->isTranslated ? NULL :
        stringBuilderToString(translator->codeBuf, outLen);
}

static int
__translateRecursive(Translator *translator, const BFCodeLex *node,
    size_t depth)
{
    const size_t *subNodes;
    const BFCodeLex *subNode;
    const BFIR *ir;

    if (node->isSection) {
        /* emit section label */
        if (translator->spec->emitSection(translator, node))
            return 1;
        /* RECURSIVE CASE */
        subNodes = (const size_t *)getElementVLArray(
            translator->syntaxTree->raw, node->children.section.seqIndex);
        for (size_t i = 0; i < node->count; i++) {
            subNode = (const BFCodeLex *)getElementVLArray(
                translator->syntaxTree->raw, subNodes[i]);
            if (__translateRecursive(translator, subNode, depth + 1))
                return 1;
        }
    } else if (node->lexType == LexType_Branch) {
        /* BASE CASE */
        /* emit jump */
        return translator->spec->emitJump(translator, node);
    } else {
        /* BASE CASE */
        ir = (const BFIR *)getElementVLArray(translator->syntaxTree->raw,
            node->children.sequence.bfCodeIndex);
        for (size_t i = 0; i < node->count; i++) {
            /* check for errors */
            if (isBasicBFKeyword(ir[i].keyword)) {
                if (translator->spec->emitBasic(translator, ir + i))
                    return 1;
            } else if (isIOBFKeyword(ir[i].keyword)) {
                if (translator->spec->emitIO(translator, ir + i))
                    return 1;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

int
pushInstruction(Translator *translator, const char *fmt, ...)
{
    va_list ap;
    static char buf[PUSH_BUF_SIZE];

    va_start(ap, fmt);
    if (!(vsprintf(buf, fmt, ap)))
        goto error1;
    if (stringBuilderPushStr(translator->codeBuf, buf))
        goto error1;
    va_end(ap);
    return 0;
error1:;
    va_end(ap);
    return 1;
}

static int
notImplementedYet()
{
    puts("Not Implemented Yet!!");
    exit(1);
    return 0;
}
