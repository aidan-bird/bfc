#ifndef TRANSLATE_LINUX_IA32_H
#define TRANSLATE_LINUX_IA32_H

#include "./translate.h"
#include "./bfc_ir.h"

int linux_ia32_emitBasic(Translator *translator, const BFIR *bfir);
int linux_ia32_emitIO(Translator *translator, const BFIR *bfir);
int linux_ia32_emitSection(Translator *translator, const BFCodeLex *node);
int linux_ia32_emitExit(Translator *translator, const BFIR *bfir);
int linux_ia32_emitStart(Translator *translator, const BFIR *bfir);
int linux_ia32_emitJump(Translator *translator, const BFCodeLex *node);

#endif
