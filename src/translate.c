#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "./translate.h"
#include "./bfc_ir.h"
#include "../alib/src/vlarray.h"
#include "../alib/src/utils.h"

#define PUSH_BUF_SIZE 32
#define TAPE_LENGTH 30000

#define linux_x86_64_pointer_reg rbx

#define SPACES "    "
#define SPACES_LEN (sizeof(SPACES) - 1)

static int linux_x86_64_emitBasic(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitIO(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitSection(Translator *translator, int sectionID);
static int linux_x86_64_emitExit(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitStart(Translator *translator, const BFIR *bfir);
static int linux_x86_64_emitJump(Translator *translator, int sectionID);
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
        // .pointerReg = "esi",
        .emitBasic = notImplementedYet,
        .emitIO = notImplementedYet,
        .emitSection = notImplementedYet,
        .emitExit = notImplementedYet,
        .emitStart = notImplementedYet,
        .emitJump = notImplementedYet,
    },
};

static const PlatformSpec *platformTabs[] = {
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
    return translator->isTranslated;
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
        translator->spec->emitSection(translator,
            node->children.section.sectionID);
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
        translator->spec->emitJump(translator,
        ((BFCodeLex *)getElementVLArray(translator->syntaxTree->raw,
            node->children.branch.sectIndex))->children.section.sectionID);
    } else {
        /* BASE CASE */
        ir = (const BFIR *)getElementVLArray(translator->syntaxTree->raw,
            node->children.sequence.bfCodeIndex);
        for (size_t i = 0; i < node->count; i++) {
            /* check for errors */
            if (isBasicBFKeyword(ir[i].keyword))
                translator->spec->emitBasic(translator, ir + i);
            else if (isIOBFKeyword(ir[i].keyword))
                translator->spec->emitIO(translator, ir + i);
            else
                return 1;
        }
    }
}

static int
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
linux_x86_64_emitBasic(Translator *translator, const BFIR *bfir)
{
    static const char *linux_x86_64_emitBasicStrsSingle[] = {
        [BFKeyword_up]   = SPACES "inc " XSTR(linux_x86_64_pointer_reg) "\n",
        [BFKeyword_down] = SPACES "dec " XSTR(linux_x86_64_pointer_reg) "\n",
        [BFKeyword_inc]  = SPACES "inc BYTE [" XSTR(linux_x86_64_pointer_reg) "] \n",
        [BFKeyword_dec]  = SPACES "dec BYTE [" XSTR(linux_x86_64_pointer_reg) "] \n",
    };
    static const char *linux_x86_64_emitBasicStrsMulti[] = {
        [BFKeyword_up]   = SPACES "add " XSTR(linux_x86_64_pointer_reg),
        [BFKeyword_down] = SPACES "sub " XSTR(linux_x86_64_pointer_reg),
        [BFKeyword_inc]  = SPACES "add BYTE [" XSTR(linux_x86_64_pointer_reg) "]",
        [BFKeyword_dec]  = SPACES "sub BYTE [" XSTR(linux_x86_64_pointer_reg) "]",
    };

    if (bfir->count == 1)
        return stringBuilderPushStr(translator->codeBuf,
            linux_x86_64_emitBasicStrsSingle[bfir->keyword]);
    return pushInstruction(translator, "%s, %d\n",
        linux_x86_64_emitBasicStrsMulti[bfir->keyword], bfir->count);
    return 0;
}

static int
linux_x86_64_emitPrintMulti(Translator *translator, size_t count)
{
    static const char printSingleCode[] =
SPACES "mov rax, 1\n"
SPACES "mov rdi, 1\n"
SPACES "mov rsi, " XSTR(linux_x86_64_pointer_reg) "\n"
SPACES "mov rdx, 1\n"
SPACES "syscall\n";
    static const char printMultiCode[] =
SPACES "mov rax, 1\n"
SPACES "syscall\n";

    if (stringBuilderPushStr(translator->codeBuf, printSingleCode))
        return 1;
    for (size_t i = 0; i < count - 1; i++) {
        /* TODO make a stringBuilder api that duplicates strings*/
        if (stringBuilderPushStr(translator->codeBuf, printMultiCode))
            return 1;
    }
    return 0;
}

static int
linux_x86_64_emitReadMulti(Translator *translator, size_t count)
{
    static const char readSingleCode[] =
SPACES "xor rax, rax\n"
SPACES "xor rdi, rdi\n"
SPACES "mov rsi, rbx\n"
SPACES "mov rdx, 1\n"
SPACES "syscall\n";
    static const char readMultiCode[] =
SPACES "xor rax, rax\n"
SPACES "syscall\n";

    if (stringBuilderPushStr(translator->codeBuf, readSingleCode))
        return 1;
    for (size_t i = 0; i < count - 1; i++) {
        /* TODO make a stringBuilder api that duplicates strings*/
        if (stringBuilderPushStr(translator->codeBuf, readMultiCode))
            return 1;
    }
    return 0;
}

static int
linux_x86_64_emitIO(Translator *translator, const BFIR *bfir)
{
    switch (bfir->keyword) {
        case BFKeyword_print:
            return linux_x86_64_emitPrintMulti(translator, bfir->count);
        case BFKeyword_read:
            return linux_x86_64_emitReadMulti(translator, bfir->count);
        default:
            return 1;
    }
    return 0;
}

static int
linux_x86_64_emitSection(Translator *translator, int sectionID)
{
    return pushInstruction(translator, "L%d:\n", sectionID);
}

static int
linux_x86_64_emitExit(Translator *translator, const BFIR *bfir)
{
    static const char exitCode[] = 
"end:\n"
SPACES "mov rax, 60\n"
SPACES "xor rdi, rdi\n"
SPACES "syscall";

    return stringBuilderPushStr(translator->codeBuf, exitCode);
}

static int
linux_x86_64_emitStart(Translator *translator, const BFIR *bfir)
{
    /* configure the program to use either the stack or malloc here */
    /* $rbx is the pointer register */
    static const char *startCode = 
"section .bss\n"
SPACES "tape: resb " XSTR(TAPE_LENGTH) "\n"
"section .text\n"
SPACES "global _start\n"
"_start:\n"
SPACES "mov rbp, rsp\n"
SPACES "mov rbx, tape\n";

    return stringBuilderPushStr(translator->codeBuf, startCode);
}

static int
linux_x86_64_emitJump(Translator *translator, int sectionID)
{
    static const char *jumpCode =
SPACES "cmp BYTE [" XSTR(linux_x86_64_pointer_reg)"], 0\n"
SPACES "jne";

    return pushInstruction(translator, "%s L%d\n", jumpCode, sectionID);
}

static int
notImplementedYet()
{
    puts("Not Implemented Yet!!");
    exit(1);
    return 0;
}
