/*
 * miniC semantic analysis.
 */

#include "semantic_analysis.h"
#include <stdio.h>
#include <set>
#include <string>
#include <vector>

using namespace std;

static bool traverse(astNode *node, vector< set<string> > *symbolTables);
static bool traverseBlockNoScope(astStmt *stmt, vector< set<string> > *symbolTables);
static bool traverseStmt(astStmt *stmt, vector< set<string> > *symbolTables);

static bool isDeclared(const vector< set<string> > &tables, const char *name) {
    for (vector< set<string> >::const_reverse_iterator it = tables.rbegin();
         it != tables.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }
    return false;
}

bool semanticAnalysis(astNode *node) {
    if (!node) {
        fprintf(stderr, "Error: AST is empty\n");
        return true;
    }

    vector< set<string> > symbolTables;
    return traverse(node, &symbolTables);
}

static bool traverse(astNode *node, vector< set<string> > *symbolTables) {
    bool errorFound = false;

    if (!node) {
        return false;
    }

    switch (node->type) {
        case ast_prog:
            errorFound = traverse(node->prog.ext1, symbolTables) || errorFound;
            errorFound = traverse(node->prog.ext2, symbolTables) || errorFound;
            errorFound = traverse(node->prog.func, symbolTables) || errorFound;
            break;

        case ast_extern:
            break;

        case ast_func: {
            set<string> currSymTable;
            symbolTables->push_back(currSymTable);

            if (node->func.param) {
                const char *pname = node->func.param->var.name;
                if (!symbolTables->back().insert(pname).second) {
                    fprintf(stderr, "Error: redeclared variable '%s'\n", pname);
                    errorFound = true;
                }
            }

            if (node->func.body && node->func.body->type == ast_stmt &&
                node->func.body->stmt.type == ast_block) {
                errorFound = traverseBlockNoScope(&node->func.body->stmt, symbolTables) || errorFound;
            } else {
                errorFound = traverse(node->func.body, symbolTables) || errorFound;
            }
            symbolTables->pop_back();
            break;
        }

        case ast_stmt:
            errorFound = traverseStmt(&node->stmt, symbolTables) || errorFound;
            break;

        case ast_var:
            if (!isDeclared(*symbolTables, node->var.name)) {
                fprintf(stderr, "Error: undeclared variable '%s'\n", node->var.name);
                errorFound = true;
            }
            break;

        case ast_cnst:
            break;

        case ast_rexpr:
            errorFound = traverse(node->rexpr.lhs, symbolTables) || errorFound;
            errorFound = traverse(node->rexpr.rhs, symbolTables) || errorFound;
            break;

        case ast_bexpr:
            errorFound = traverse(node->bexpr.lhs, symbolTables) || errorFound;
            errorFound = traverse(node->bexpr.rhs, symbolTables) || errorFound;
            break;

        case ast_uexpr:
            errorFound = traverse(node->uexpr.expr, symbolTables) || errorFound;
            break;
    }

    return errorFound;
}

static bool traverseStmt(astStmt *stmt, vector< set<string> > *symbolTables) {
    bool errorFound = false;

    if (!stmt) {
        return false;
    }

    switch (stmt->type) {
        case ast_call:
            if (stmt->call.param) {
                errorFound = traverse(stmt->call.param, symbolTables) || errorFound;
            }
            break;

        case ast_ret:
            errorFound = traverse(stmt->ret.expr, symbolTables) || errorFound;
            break;

        case ast_block: {
            set<string> currSymTable;
            symbolTables->push_back(currSymTable);

            for (size_t i = 0; i < stmt->block.stmt_list->size(); ++i) {
                errorFound = traverse((*stmt->block.stmt_list)[i], symbolTables) || errorFound;
            }

            symbolTables->pop_back();
            break;
        }

        case ast_while:
            errorFound = traverse(stmt->whilen.cond, symbolTables) || errorFound;
            errorFound = traverseStmt(&stmt->whilen.body->stmt, symbolTables) || errorFound;
            break;

        case ast_if:
            errorFound = traverse(stmt->ifn.cond, symbolTables) || errorFound;
            errorFound = traverseStmt(&stmt->ifn.if_body->stmt, symbolTables) || errorFound;
            if (stmt->ifn.else_body) {
                errorFound = traverseStmt(&stmt->ifn.else_body->stmt, symbolTables) || errorFound;
            }
            break;

        case ast_asgn:
            errorFound = traverse(stmt->asgn.lhs, symbolTables) || errorFound;
            errorFound = traverse(stmt->asgn.rhs, symbolTables) || errorFound;
            break;

        case ast_decl:
            if (!symbolTables->back().insert(stmt->decl.name).second) {
                fprintf(stderr, "Error: redeclared variable '%s'\n", stmt->decl.name);
                errorFound = true;
            }
            break;
    }

    return errorFound;
}

static bool traverseBlockNoScope(astStmt *stmt, vector< set<string> > *symbolTables) {
    bool errorFound = false;

    if (!stmt || stmt->type != ast_block) {
        return false;
    }

    for (size_t i = 0; i < stmt->block.stmt_list->size(); ++i) {
        errorFound = traverse((*stmt->block.stmt_list)[i], symbolTables) || errorFound;
    }

    return errorFound;
}
