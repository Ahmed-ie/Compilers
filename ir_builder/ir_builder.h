#ifndef IR_BUILDER_H
#define IR_BUILDER_H

#include <llvm-c/Core.h>
#include "../frontend/ast.h"

// Renames declared variables and their uses so each declaration has a unique name.
void renameVariables(astNode *root);

// Builds an LLVM module from the (already semantically valid) AST.
LLVMModuleRef buildModuleFromAst(astNode *root);

#endif
