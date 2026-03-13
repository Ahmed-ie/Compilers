/*
 * MiniC LLVM IR Builder
 * Author: Ahmed Elmi
 * Course: CS57
 *
 * Implementation highlights:
 * - Prepass renames declarations/uses to unique names.
 * - Function codegen uses entry allocas and a unified return block.
 * - Statement codegen builds explicit CFG for if/while constructs.
 */

#include "ir_builder.h"

#include <llvm-c/Core.h>

#include <cstdlib>
#include <cstring>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

char *dupCStr(const std::string &s) {
    char *out = static_cast<char *>(std::calloc(s.size() + 1, sizeof(char)));
    if (!out) {
        return nullptr;
    }
    std::memcpy(out, s.c_str(), s.size());
    out[s.size()] = '\0';
    return out;
}

void replaceName(char *&slot, const std::string &value) {
    std::free(slot);
    slot = dupCStr(value);
}

class Renamer {
  public:
    void run(astNode *root) {
        if (!root) {
            return;
        }
        visitNode(root);
    }

  private:
    std::vector<std::unordered_map<std::string, std::string> > scopes_;
    int counter_ = 0;

    void pushScope() { scopes_.push_back(std::unordered_map<std::string, std::string>()); }
    void popScope() { scopes_.pop_back(); }

    std::string makeUnique(const std::string &name) { return name + "__" + std::to_string(counter_++); }

    bool resolve(const std::string &name, std::string *mapped) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto f = it->find(name);
            if (f != it->end()) {
                *mapped = f->second;
                return true;
            }
        }
        return false;
    }

    void renameDecl(char *&nameSlot) {
        // Give each declaration its own name so nested scopes do not collide later.
        const std::string oldName(nameSlot);
        const std::string unique = makeUnique(oldName);
        scopes_.back()[oldName] = unique;
        replaceName(nameSlot, unique);
    }

    void renameVarUse(char *&nameSlot) {
        std::string mapped;
        if (resolve(nameSlot, &mapped)) {
            // Rewrite uses to the renamed declaration they actually refer to.
            replaceName(nameSlot, mapped);
        }
    }

    void visitStmt(astStmt *stmt) {
        if (!stmt) {
            return;
        }
        switch (stmt->type) {
            case ast_call:
                if (stmt->call.param) {
                    visitNode(stmt->call.param);
                }
                break;
            case ast_ret:
                visitNode(stmt->ret.expr);
                break;
            case ast_block:
                pushScope();
                for (astNode *child : *stmt->block.stmt_list) {
                    visitNode(child);
                }
                popScope();
                break;
            case ast_while:
                visitNode(stmt->whilen.cond);
                visitNode(stmt->whilen.body);
                break;
            case ast_if:
                visitNode(stmt->ifn.cond);
                visitNode(stmt->ifn.if_body);
                if (stmt->ifn.else_body) {
                    visitNode(stmt->ifn.else_body);
                }
                break;
            case ast_asgn:
                visitNode(stmt->asgn.lhs);
                visitNode(stmt->asgn.rhs);
                break;
            case ast_decl:
                renameDecl(stmt->decl.name);
                break;
        }
    }

    void visitNode(astNode *node) {
        if (!node) {
            return;
        }
        switch (node->type) {
            case ast_prog:
                visitNode(node->prog.ext1);
                visitNode(node->prog.ext2);
                visitNode(node->prog.func);
                break;
            case ast_func:
                pushScope();
                if (node->func.param) {
                    renameDecl(node->func.param->var.name);
                }
                visitNode(node->func.body);
                popScope();
                break;
            case ast_stmt:
                visitStmt(&node->stmt);
                break;
            case ast_var:
                renameVarUse(node->var.name);
                break;
            case ast_cnst:
                break;
            case ast_rexpr:
                visitNode(node->rexpr.lhs);
                visitNode(node->rexpr.rhs);
                break;
            case ast_bexpr:
                visitNode(node->bexpr.lhs);
                visitNode(node->bexpr.rhs);
                break;
            case ast_uexpr:
                visitNode(node->uexpr.expr);
                break;
            case ast_extern:
                break;
        }
    }
};

struct BuildContext {
    LLVMModuleRef module = nullptr;
    LLVMBuilderRef builder = nullptr;
    LLVMValueRef currentFunc = nullptr;
    LLVMValueRef readFn = nullptr;
    LLVMValueRef printFn = nullptr;
    LLVMTypeRef readFnType = nullptr;
    LLVMTypeRef printFnType = nullptr;
    std::unordered_map<std::string, LLVMValueRef> varMap;
    LLVMValueRef retAlloca = nullptr;
    LLVMBasicBlockRef retBB = nullptr;
};

// Collect all declared variable names so allocas can be emitted in function entry.
void collectDeclNamesFromStmt(astNode *node, std::set<std::string> *names) {
    if (!node || node->type != ast_stmt) {
        return;
    }
    astStmt &stmt = node->stmt;
    switch (stmt.type) {
        case ast_decl:
            names->insert(stmt.decl.name);
            break;
        case ast_block:
            for (astNode *child : *stmt.block.stmt_list) {
                collectDeclNamesFromStmt(child, names);
            }
            break;
        case ast_if:
            collectDeclNamesFromStmt(stmt.ifn.if_body, names);
            if (stmt.ifn.else_body) {
                collectDeclNamesFromStmt(stmt.ifn.else_body, names);
            }
            break;
        case ast_while:
            collectDeclNamesFromStmt(stmt.whilen.body, names);
            break;
        case ast_call:
        case ast_ret:
        case ast_asgn:
            break;
    }
}

LLVMIntPredicate mapRelOp(rop_type op) {
    switch (op) {
        case lt:
            return LLVMIntSLT;
        case gt:
            return LLVMIntSGT;
        case le:
            return LLVMIntSLE;
        case ge:
            return LLVMIntSGE;
        case eq:
            return LLVMIntEQ;
        case neq:
            return LLVMIntNE;
    }
    return LLVMIntEQ;
}

LLVMValueRef genExpr(astNode *expr, BuildContext *ctx);
LLVMBasicBlockRef genStmt(astNode *stmtNode, LLVMBasicBlockRef startBB, BuildContext *ctx);

LLVMValueRef getVarPtr(const char *name, BuildContext *ctx) {
    auto it = ctx->varMap.find(name);
    if (it == ctx->varMap.end()) {
        return nullptr;
    }
    return it->second;
}

void branchIfUnterminated(LLVMBuilderRef builder, LLVMBasicBlockRef from, LLVMBasicBlockRef to) {
    // Avoid creating malformed IR with duplicate terminators.
    if (!LLVMGetBasicBlockTerminator(from)) {
        LLVMPositionBuilderAtEnd(builder, from);
        LLVMBuildBr(builder, to);
    }
}

LLVMValueRef genExpr(astNode *expr, BuildContext *ctx) {
    if (!expr) {
        return LLVMConstInt(LLVMInt32Type(), 0, 1);
    }

    switch (expr->type) {
        case ast_cnst:
            return LLVMConstInt(LLVMInt32Type(), static_cast<unsigned long long>(expr->cnst.value), 1);
        case ast_var: {
            LLVMValueRef ptr = getVarPtr(expr->var.name, ctx);
            return LLVMBuildLoad2(ctx->builder, LLVMInt32Type(), ptr, "loadtmp");
        }
        case ast_uexpr: {
            LLVMValueRef v = genExpr(expr->uexpr.expr, ctx);
            LLVMValueRef zero = LLVMConstInt(LLVMInt32Type(), 0, 1);
            return LLVMBuildSub(ctx->builder, zero, v, "negtmp");
        }
        case ast_bexpr: {
            LLVMValueRef lhs = genExpr(expr->bexpr.lhs, ctx);
            LLVMValueRef rhs = genExpr(expr->bexpr.rhs, ctx);
            switch (expr->bexpr.op) {
                case add:
                    return LLVMBuildAdd(ctx->builder, lhs, rhs, "addtmp");
                case sub:
                    return LLVMBuildSub(ctx->builder, lhs, rhs, "subtmp");
                case mul:
                    return LLVMBuildMul(ctx->builder, lhs, rhs, "multmp");
                case divide:
                    return LLVMBuildSDiv(ctx->builder, lhs, rhs, "divtmp");
                case uminus:
                    return LLVMBuildSub(ctx->builder, LLVMConstInt(LLVMInt32Type(), 0, 1), rhs, "negtmp");
            }
            break;
        }
        case ast_rexpr: {
            LLVMValueRef lhs = genExpr(expr->rexpr.lhs, ctx);
            LLVMValueRef rhs = genExpr(expr->rexpr.rhs, ctx);
            return LLVMBuildICmp(ctx->builder, mapRelOp(expr->rexpr.op), lhs, rhs, "cmptmp");
        }
        case ast_stmt:
            if (expr->stmt.type == ast_call && std::strcmp(expr->stmt.call.name, "read") == 0) {
                return LLVMBuildCall2(ctx->builder, ctx->readFnType, ctx->readFn, nullptr, 0, "readcall");
            }
            break;
        case ast_prog:
        case ast_func:
        case ast_extern:
            break;
    }

    return LLVMConstInt(LLVMInt32Type(), 0, 1);
}

LLVMBasicBlockRef genStmt(astNode *stmtNode, LLVMBasicBlockRef startBB, BuildContext *ctx) {
    if (!stmtNode || stmtNode->type != ast_stmt) {
        return startBB;
    }

    astStmt &stmt = stmtNode->stmt;
    switch (stmt.type) {
        case ast_decl:
            return startBB;

        case ast_asgn: {
            LLVMPositionBuilderAtEnd(ctx->builder, startBB);
            LLVMValueRef rhs = genExpr(stmt.asgn.rhs, ctx);
            LLVMValueRef ptr = getVarPtr(stmt.asgn.lhs->var.name, ctx);
            LLVMBuildStore(ctx->builder, rhs, ptr);
            return startBB;
        }

        case ast_call: {
            LLVMPositionBuilderAtEnd(ctx->builder, startBB);
            if (std::strcmp(stmt.call.name, "print") == 0) {
                LLVMValueRef arg = genExpr(stmt.call.param, ctx);
                LLVMValueRef args[] = {arg};
                LLVMBuildCall2(ctx->builder, ctx->printFnType, ctx->printFn, args, 1, "");
            } else if (std::strcmp(stmt.call.name, "read") == 0) {
                LLVMBuildCall2(ctx->builder, ctx->readFnType, ctx->readFn, nullptr, 0, "");
            }
            return startBB;
        }

        case ast_ret: {
            LLVMPositionBuilderAtEnd(ctx->builder, startBB);
            LLVMValueRef retVal = genExpr(stmt.ret.expr, ctx);
            // Return statements store into ret slot, then jump to the shared ret block.
            LLVMBuildStore(ctx->builder, retVal, ctx->retAlloca);
            LLVMBuildBr(ctx->builder, ctx->retBB);
            // Return a fresh block for sequencing, then prune unreachable blocks later.
            return LLVMAppendBasicBlock(ctx->currentFunc, "after_ret");
        }

        case ast_block: {
            LLVMBasicBlockRef prev = startBB;
            for (astNode *child : *stmt.block.stmt_list) {
                prev = genStmt(child, prev, ctx);
            }
            return prev;
        }

        case ast_if: {
            LLVMPositionBuilderAtEnd(ctx->builder, startBB);
            LLVMValueRef cond = genExpr(stmt.ifn.cond, ctx);
            // CFG shape: start -> {if_true, if_false} -> optional if_end.
            LLVMBasicBlockRef trueBB = LLVMAppendBasicBlock(ctx->currentFunc, "if_true");
            LLVMBasicBlockRef falseBB = LLVMAppendBasicBlock(ctx->currentFunc, "if_false");
            LLVMBuildCondBr(ctx->builder, cond, trueBB, falseBB);

            if (!stmt.ifn.else_body) {
                LLVMBasicBlockRef ifExit = genStmt(stmt.ifn.if_body, trueBB, ctx);
                branchIfUnterminated(ctx->builder, ifExit, falseBB);
                return falseBB;
            }

            LLVMBasicBlockRef ifExit = genStmt(stmt.ifn.if_body, trueBB, ctx);
            LLVMBasicBlockRef elseExit = genStmt(stmt.ifn.else_body, falseBB, ctx);
            LLVMBasicBlockRef endBB = LLVMAppendBasicBlock(ctx->currentFunc, "if_end");
            branchIfUnterminated(ctx->builder, ifExit, endBB);
            branchIfUnterminated(ctx->builder, elseExit, endBB);
            return endBB;
        }

        case ast_while: {
            LLVMPositionBuilderAtEnd(ctx->builder, startBB);
            // CFG shape: start -> while_cond -> {while_body, while_end}, body loops back to cond.
            LLVMBasicBlockRef condBB = LLVMAppendBasicBlock(ctx->currentFunc, "while_cond");
            LLVMBasicBlockRef bodyBB = LLVMAppendBasicBlock(ctx->currentFunc, "while_body");
            LLVMBasicBlockRef endBB = LLVMAppendBasicBlock(ctx->currentFunc, "while_end");
            LLVMBuildBr(ctx->builder, condBB);

            LLVMPositionBuilderAtEnd(ctx->builder, condBB);
            LLVMValueRef cond = genExpr(stmt.whilen.cond, ctx);
            LLVMBuildCondBr(ctx->builder, cond, bodyBB, endBB);

            LLVMBasicBlockRef bodyExit = genStmt(stmt.whilen.body, bodyBB, ctx);
            branchIfUnterminated(ctx->builder, bodyExit, condBB);
            return endBB;
        }
    }

    return startBB;
}

void removeUnreachableBlocks(LLVMValueRef func) {
    LLVMBasicBlockRef entry = LLVMGetEntryBasicBlock(func);
    if (!entry) {
        return;
    }

    std::queue<LLVMBasicBlockRef> q;
    std::unordered_set<LLVMBasicBlockRef> reachable;
    q.push(entry);
    reachable.insert(entry);

    // BFS from entry to identify reachable blocks.
    while (!q.empty()) {
        LLVMBasicBlockRef bb = q.front();
        q.pop();

        LLVMValueRef term = LLVMGetBasicBlockTerminator(bb);
        if (!term) {
            continue;
        }
        unsigned n = LLVMGetNumSuccessors(term);
        for (unsigned i = 0; i < n; ++i) {
            LLVMBasicBlockRef succ = LLVMGetSuccessor(term, i);
            if (reachable.insert(succ).second) {
                q.push(succ);
            }
        }
    }

    std::vector<LLVMBasicBlockRef> dead;
    for (LLVMBasicBlockRef bb = LLVMGetFirstBasicBlock(func); bb; bb = LLVMGetNextBasicBlock(bb)) {
        if (!reachable.count(bb)) {
            dead.push_back(bb);
        }
    }

    for (LLVMBasicBlockRef bb : dead) {
        LLVMDeleteBasicBlock(bb);
    }
}

LLVMValueRef buildFunction(astNode *funcNode, BuildContext *ctx) {
    unsigned paramCount = (funcNode->func.param != nullptr) ? 1U : 0U;
    LLVMTypeRef params[1] = {LLVMInt32Type()};
    LLVMTypeRef fnType = LLVMFunctionType(LLVMInt32Type(), params, paramCount, 0);
    LLVMValueRef fn = LLVMAddFunction(ctx->module, funcNode->func.name, fnType);

    ctx->currentFunc = fn;
    ctx->builder = LLVMCreateBuilder();
    ctx->varMap.clear();

    LLVMBasicBlockRef entryBB = LLVMAppendBasicBlock(fn, "entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entryBB);

    std::set<std::string> names;
    if (funcNode->func.param) {
        names.insert(funcNode->func.param->var.name);
    }
    collectDeclNamesFromStmt(funcNode->func.body, &names);

    // Emit all local/parameter allocas in entry as required by assignment.
    for (const std::string &name : names) {
        LLVMValueRef allocaInst = LLVMBuildAlloca(ctx->builder, LLVMInt32Type(), name.c_str());
        LLVMSetAlignment(allocaInst, 4);
        ctx->varMap[name] = allocaInst;
    }

    // Dedicated slot for function result used by all return statements.
    ctx->retAlloca = LLVMBuildAlloca(ctx->builder, LLVMInt32Type(), "ret_slot");
    LLVMSetAlignment(ctx->retAlloca, 4);
    LLVMBuildStore(ctx->builder, LLVMConstInt(LLVMInt32Type(), 0, 1), ctx->retAlloca);

    if (funcNode->func.param) {
        LLVMBuildStore(ctx->builder, LLVMGetParam(fn, 0), ctx->varMap[funcNode->func.param->var.name]);
    }

    ctx->retBB = LLVMAppendBasicBlock(fn, "ret");
    LLVMPositionBuilderAtEnd(ctx->builder, ctx->retBB);
    // Unified exit: load final return value and emit `ret`.
    LLVMValueRef rv = LLVMBuildLoad2(ctx->builder, LLVMInt32Type(), ctx->retAlloca, "retval");
    LLVMBuildRet(ctx->builder, rv);

    LLVMBasicBlockRef exitBB = genStmt(funcNode->func.body, entryBB, ctx);
    branchIfUnterminated(ctx->builder, exitBB, ctx->retBB);

    removeUnreachableBlocks(fn);
    LLVMDisposeBuilder(ctx->builder);
    ctx->builder = nullptr;
    return fn;
}

}  // namespace

void renameVariables(astNode *root) {
    Renamer renamer;
    renamer.run(root);
}

LLVMModuleRef buildModuleFromAst(astNode *root) {
    LLVMModuleRef module = LLVMModuleCreateWithName("minic_module");
    LLVMSetTarget(module, "x86_64-pc-linux-gnu");

    BuildContext ctx;
    ctx.module = module;

    LLVMTypeRef readParams[] = {};
    ctx.readFnType = LLVMFunctionType(LLVMInt32Type(), readParams, 0, 0);
    ctx.readFn = LLVMAddFunction(module, "read", ctx.readFnType);

    LLVMTypeRef printParams[] = {LLVMInt32Type()};
    ctx.printFnType = LLVMFunctionType(LLVMVoidType(), printParams, 1, 0);
    ctx.printFn = LLVMAddFunction(module, "print", ctx.printFnType);

    if (root && root->type == ast_prog && root->prog.func && root->prog.func->type == ast_func) {
        buildFunction(root->prog.func, &ctx);
    }

    return module;
}
