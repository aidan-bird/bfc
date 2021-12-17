#include "../alib/src/utils.h"
#include "./config.h"
#include "./translate.h"
#include "./translate_linux_x86_64.h"

int
linux_x86_64_emitBasic(Translator *translator, const BFIR *bfir)
{
    static const char *linux_x86_64_emitBasicStrsSingle[] = {
        [BFKeyword_up] = SPACES "inc " XSTR(LINUX_X86_64_POINTER_REG) "\n",
        [BFKeyword_down] = SPACES "dec " XSTR(LINUX_X86_64_POINTER_REG) "\n",
        [BFKeyword_inc] = 
            SPACES "inc BYTE [" XSTR(LINUX_X86_64_POINTER_REG) "] \n",
        [BFKeyword_dec] =
            SPACES "dec BYTE [" XSTR(LINUX_X86_64_POINTER_REG) "] \n",
    };
    static const char *linux_x86_64_emitBasicStrsMulti[] = {
        [BFKeyword_up] = SPACES "add " XSTR(LINUX_X86_64_POINTER_REG),
        [BFKeyword_down] = SPACES "sub " XSTR(LINUX_X86_64_POINTER_REG),
        [BFKeyword_inc] =
            SPACES "add BYTE [" XSTR(LINUX_X86_64_POINTER_REG) "]",
        [BFKeyword_dec] =
            SPACES "sub BYTE [" XSTR(LINUX_X86_64_POINTER_REG) "]",
    };

    if (bfir->count == 1)
        return stringBuilderPushStr(translator->codeBuf,
            linux_x86_64_emitBasicStrsSingle[bfir->keyword]);
    return pushInstruction(translator, "%s, %d\n",
        linux_x86_64_emitBasicStrsMulti[bfir->keyword], bfir->count);
    return 0;
}

int
linux_x86_64_emitPrintMulti(Translator *translator, size_t count)
{
    static const char printSingleCode[] =
SPACES "mov rax, 1\n"
SPACES "mov rdi, 1\n"
SPACES "mov rsi, " XSTR(LINUX_X86_64_POINTER_REG) "\n"
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

int
linux_x86_64_emitReadMulti(Translator *translator, size_t count)
{
    static const char readSingleCode[] =
SPACES "xor rax, rax\n"
SPACES "xor rdi, rdi\n"
SPACES "mov rsi, " XSTR(LINUX_X86_64_POINTER_REG) "\n"
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

int
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

int
linux_x86_64_emitSection(Translator *translator, const BFCodeLex *sectionNode)
{
    static const char *sectionCode =
SPACES "cmp BYTE [" XSTR(LINUX_X86_64_POINTER_REG)"], 0\n"
SPACES "je";

    if (sectionNode->children.section.sectionID == ROOT_SECTION_ID)
        return stringBuilderPushStr(translator->codeBuf, "L0:\n");
    return pushInstruction(translator, "L%d:\n",
        sectionNode->children.section.sectionID)
    || stringBuilderPushStr(translator->codeBuf, sectionCode)
    || pushInstruction(translator, " L%d\n",
        ((BFCodeLex *)getElementVLArray(translator->syntaxTree->raw,
        sectionNode->children.section.endIndex))->children.branch.sectionID);
}

int
linux_x86_64_emitExit(Translator *translator, const BFIR *bfir)
{
    static const char exitCode[] = 
"end:\n"
SPACES "mov rax, 60\n"
SPACES "xor rdi, rdi\n"
SPACES "syscall";

    return stringBuilderPushStr(translator->codeBuf, exitCode);
}

int
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
SPACES "mov " XSTR(LINUX_X86_64_POINTER_REG) ", tape\n";

    return stringBuilderPushStr(translator->codeBuf, startCode);
}

int
linux_x86_64_emitJump(Translator *translator, const BFCodeLex *node)
{
    static const char *jumpCode =
SPACES "cmp BYTE [" XSTR(LINUX_X86_64_POINTER_REG)"], 0\n"
SPACES "jne";

    return pushInstruction(translator, "%s L%d\nL%d:\n", jumpCode, 
        ((BFCodeLex *)getElementVLArray(translator->syntaxTree->raw,
        node->children.branch.sectIndex))->children.section.sectionID,
        node->children.branch.sectionID);
}
