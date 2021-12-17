#ifndef TRANSLATE_LINUX_X86_64_H
#define TRANSLATE_LINUX_X86_64_H

#include "./translate.h"
#include "./bfc_ir.h"

int linux_x86_64_emitBasic(Translator *translator, const BFIR *bfir);
int linux_x86_64_emitIO(Translator *translator, const BFIR *bfir);
int linux_x86_64_emitSection(Translator *translator, const BFCodeLex *node);
int linux_x86_64_emitExit(Translator *translator, const BFIR *bfir);
int linux_x86_64_emitStart(Translator *translator, const BFIR *bfir);
int linux_x86_64_emitJump(Translator *translator, const BFCodeLex *node);

#endif
