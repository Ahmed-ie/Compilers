#ifndef IR_BUILDER_H
#define IR_BUILDER_H

#include <llvm-c/Core.h>
#include "../frontend/ast.h"

/*
 * MiniC IR Builder Interface
 * Author: Ahmed Elmi
 * Course: CS57
 *
 * This stage expects a semantically valid AST.
 */

// Rename every declaration/use so each variable name becomes unique in the AST.
void renameVariables(astNode *root);

// Build an LLVM module for the MiniC program rooted at `root`.
LLVMModuleRef buildModuleFromAst(astNode *root);

#endif
