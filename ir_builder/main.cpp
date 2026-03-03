#include <cstdio>
#include <cstdlib>

#include <llvm-c/Core.h>

#include "../frontend/ast.h"
#include "../frontend/semantic_analysis.h"
#include "ir_builder.h"
#include "yac.tab.h"

extern int yyparse(void);
extern FILE *yyin;
extern int yylex_destroy(void);
extern astNode *root;

static void cleanup() {
    if (yyin && yyin != stdin) {
        fclose(yyin);
        yyin = nullptr;
    }
    yylex_destroy();
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        std::fprintf(stderr, "Usage: %s <input.c> [output.ll]\n", argv[0]);
        return 1;
    }

    yyin = std::fopen(argv[1], "r");
    if (!yyin) {
        std::fprintf(stderr, "Could not open file '%s'\n", argv[1]);
        return 1;
    }

    if (yyparse() != 0) {
        std::fprintf(stderr, "Parsing unsuccessful.\n");
        cleanup();
        return 2;
    }

    if (!root) {
        std::fprintf(stderr, "Error: AST is empty\n");
        cleanup();
        return 2;
    }

    if (semanticAnalysis(root)) {
        std::fprintf(stderr, "Semantic analysis unsuccessful.\n");
        cleanup();
        freeNode(root);
        return 3;
    }

    renameVariables(root);
    LLVMModuleRef module = buildModuleFromAst(root);

    char *err = nullptr;
    if (argc == 3) {
        if (LLVMPrintModuleToFile(module, argv[2], &err) != 0) {
            std::fprintf(stderr, "Failed to write LLVM IR: %s\n", err ? err : "unknown error");
            if (err) {
                LLVMDisposeMessage(err);
            }
            LLVMDisposeModule(module);
            cleanup();
            freeNode(root);
            return 4;
        }
    } else {
        char *ir = LLVMPrintModuleToString(module);
        if (ir) {
            std::printf("%s", ir);
            LLVMDisposeMessage(ir);
        }
    }

    LLVMDisposeModule(module);
    cleanup();
    freeNode(root);
    return 0;
}
