#include "../alib/src/utils.h"
#include "./config.h"
#include "./translate.h"
#include "./translate_linux_ia32.h"

int
linux_ia32_emitBasic(Translator *translator, const BFIR *bfir)
{
    static const char *linux_ia32_emitBasicStrsSingle[] = {
        [BFKeyword_up] = SPACES "inc " XSTR(LINUX_IA32_POINTER_REG) "\n",
        [BFKeyword_down] = SPACES "dec " XSTR(LINUX_IA32_POINTER_REG) "\n",
        [BFKeyword_inc] = 
            SPACES "inc BYTE [" XSTR(LINUX_IA32_POINTER_REG) "] \n",
        [BFKeyword_dec] =
            SPACES "dec BYTE [" XSTR(LINUX_IA32_POINTER_REG) "] \n",
    };
    static const char *linux_ia32_emitBasicStrsMulti[] = {
        [BFKeyword_up] = SPACES "add " XSTR(LINUX_IA32_POINTER_REG),
        [BFKeyword_down] = SPACES "sub " XSTR(LINUX_IA32_POINTER_REG),
        [BFKeyword_inc] =
            SPACES "add BYTE [" XSTR(LINUX_IA32_POINTER_REG) "]",
        [BFKeyword_dec] =
            SPACES "sub BYTE [" XSTR(LINUX_IA32_POINTER_REG) "]",
    };

    if (bfir->count == 1)
        return stringBuilderPushStr(translator->codeBuf,
            linux_ia32_emitBasicStrsSingle[bfir->keyword]);
    return pushInstruction(translator, "%s, %d\n",
        linux_ia32_emitBasicStrsMulti[bfir->keyword], bfir->count);
    return 0;
}

int
linux_ia32_emitPrintMulti(Translator *translator, size_t count)
{
    static const char printSingleCode[] =
SPACES "mov eax, 4\n"
SPACES "mov ebx, 1\n"
SPACES "mov ecx, " XSTR(LINUX_IA32_POINTER_REG) "\n"
SPACES "int 0x80\n";
    static const char printMultiCode[] =
SPACES "mov eax, 4\n"
SPACES "int 0x80\n";

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
linux_ia32_emitReadMulti(Translator *translator, size_t count)
{
    static const char readSingleCode[] =
SPACES "xor ebx, ebx\n"
SPACES "mov eax, 3\n"
SPACES "mov ecx, " XSTR(LINUX_IA32_POINTER_REG) "\n"
SPACES "int 0x80\n";
    static const char readMultiCode[] =
SPACES "mov eax, 3\n"
SPACES "int 0x80\n";

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
linux_ia32_emitIO(Translator *translator, const BFIR *bfir)
{
    switch (bfir->keyword) {
        case BFKeyword_print:
            return linux_ia32_emitPrintMulti(translator, bfir->count);
        case BFKeyword_read:
            return linux_ia32_emitReadMulti(translator, bfir->count);
        default:
            return 1;
    }
    return 0;
}

int
linux_ia32_emitSection(Translator *translator, const BFCodeLex *sectionNode)
{
    static const char *sectionCode =
SPACES "cmp BYTE [" XSTR(LINUX_IA32_POINTER_REG)"], 0\n"
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
linux_ia32_emitExit(Translator *translator, const BFIR *bfir)
{
    static const char exitCode[] = 
"end:\n"
SPACES "mov eax, 1\n"
SPACES "xor ebx, ebx\n"
SPACES "int 0x80";

    return stringBuilderPushStr(translator->codeBuf, exitCode);
}

int
linux_ia32_emitStart(Translator *translator, const BFIR *bfir)
{
    /* configure the program to use either the stack or malloc here */
    /* $rbx is the pointer register */
    static const char *startCode = 
"section .bss\n"
SPACES "tape: resb " XSTR(TAPE_LENGTH) "\n"
"section .text\n"
SPACES "global _start\n"
"_start:\n"
SPACES "mov ebp, esp\n"
SPACES "mov " XSTR(LINUX_IA32_POINTER_REG) ", tape\n"
SPACES "mov edx, 1\n";

    return stringBuilderPushStr(translator->codeBuf, startCode);
}

int
linux_ia32_emitJump(Translator *translator, const BFCodeLex *node)
{
    static const char *jumpCode =
SPACES "cmp BYTE [" XSTR(LINUX_IA32_POINTER_REG)"], 0\n"
SPACES "jne";

    return pushInstruction(translator, "%s L%d\nL%d:\n", jumpCode, 
        ((BFCodeLex *)getElementVLArray(translator->syntaxTree->raw,
        node->children.branch.sectIndex))->children.section.sectionID,
        node->children.branch.sectionID);
}
