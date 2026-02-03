/*
 * Minimal driver for MiniC frontend (parse only).
 */

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "semantic_analysis.h"
#include "yac.tab.h"

extern int yyparse(void);
extern FILE *yyin;
extern int yylex_destroy(void);
extern int yylineno;
extern char *yytext;
extern astNode *root;

static void cleanup(void) {
    if (yyin && yyin != stdin) {
        fclose(yyin);
    }
    yylex_destroy();
}

int main(int argc, char **argv) {
    if (argc == 2) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Could not open file '%s'\n", argv[1]);
            return 1;
        }
    }

    if (yyparse() != 0) {
        fprintf(stderr, "Parsing unsuccessful.\n");
        cleanup();
        return 2;
    }

    printf("Parsing successful.\n");
#ifdef DEBUG
    if (root) {
        printNode(root, 0);
    }
#endif

    if (root) {
        bool errorFound = semanticAnalysis(root);
        if (errorFound) {
            fprintf(stderr, "Semantic analysis unsuccessful.\n");
            cleanup();
            freeNode(root);
            return 3;
        }
        printf("Semantic analysis successful.\n");
    }

    cleanup();
    if (root) {
        freeNode(root);
    }
    return 0;
}
