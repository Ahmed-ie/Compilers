/*
 * MiniC Backend
 * Author: Ahmed Elmi
 * Course: CS57
 *
 * Input: LLVM IR
 * Output: x86 32-bit assembly using the assignment's local register allocation rules.
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/SourceMgr.h>

namespace {

enum PhysReg {
    REG_EBX = 0,
    REG_ECX = 1,
    REG_EDX = 2,
    REG_NONE = -1,
};

const char *regName(int reg) {
    switch (reg) {
        case REG_EBX:
            return "%ebx";
        case REG_ECX:
            return "%ecx";
        case REG_EDX:
            return "%edx";
        default:
            return "%eax";
    }
}

bool isIgnoredAlloca(const llvm::Instruction *inst) {
    return llvm::isa<llvm::AllocaInst>(inst);
}

bool isConstantInt(const llvm::Value *value) {
    return llvm::isa<llvm::ConstantInt>(value);
}

int getConstantInt(const llvm::Value *value) {
    return static_cast<int>(llvm::cast<llvm::ConstantInt>(value)->getSExtValue());
}

bool hasResult(const llvm::Instruction *inst) {
    return !inst->getType()->isVoidTy() && !isIgnoredAlloca(inst);
}

bool isArithmeticOp(const llvm::Instruction *inst) {
    return llvm::isa<llvm::BinaryOperator>(inst) &&
           (inst->getOpcode() == llvm::Instruction::Add ||
            inst->getOpcode() == llvm::Instruction::Sub ||
            inst->getOpcode() == llvm::Instruction::Mul);
}

bool isValueDefinedInBlock(const llvm::Value *value, const llvm::BasicBlock *bb) {
    const auto *inst = llvm::dyn_cast<llvm::Instruction>(value);
    return inst && inst->getParent() == bb && !isIgnoredAlloca(inst);
}

std::string jumpMnemonic(llvm::CmpInst::Predicate pred) {
    switch (pred) {
        case llvm::CmpInst::ICMP_SLT:
            return "jl";
        case llvm::CmpInst::ICMP_SGT:
            return "jg";
        case llvm::CmpInst::ICMP_SLE:
            return "jle";
        case llvm::CmpInst::ICMP_SGE:
            return "jge";
        case llvm::CmpInst::ICMP_EQ:
            return "je";
        case llvm::CmpInst::ICMP_NE:
            return "jne";
        default:
            return "je";
    }
}

std::string arithMnemonic(unsigned opcode) {
    switch (opcode) {
        case llvm::Instruction::Add:
            return "addl";
        case llvm::Instruction::Sub:
            return "subl";
        case llvm::Instruction::Mul:
            return "imull";
        default:
            return "addl";
    }
}

struct Backend {
    explicit Backend(std::ostream &out_stream) : out(out_stream) {}

    std::ostream &out;
    std::unordered_map<const llvm::BasicBlock *, std::string> bb_labels;
    std::unordered_map<const llvm::Value *, int> offset_map;
    std::unordered_map<const llvm::Instruction *, int> reg_map;
    std::unordered_map<const llvm::Instruction *, int> inst_index;
    std::unordered_map<const llvm::Instruction *, std::pair<int, int> > live_range;
    int local_mem = 0;
    void emitLine(const std::string &line) { out << line << "\n"; }

    void createBBLabels(const llvm::Function &func) {
        bb_labels.clear();
        int id = 0;
        for (const llvm::BasicBlock &bb : func) {
            bb_labels[&bb] = ".L" + func.getName().str() + "_" + std::to_string(id++);
        }
    }

    void printDirectives(const llvm::Function &func) {
        emitLine(".text");
        emitLine(".globl " + func.getName().str());
        emitLine(".type " + func.getName().str() + ", @function");
        emitLine(func.getName().str() + ":");
    }

    void printFunctionEnd() {
        emitLine("\tleave");
        emitLine("\tret");
    }

    int allocateTempSlot(const llvm::Value *value) {
        auto it = offset_map.find(value);
        if (it != offset_map.end()) {
            return it->second;
        }
        local_mem += 4;
        int offset = -local_mem;
        offset_map[value] = offset;
        return offset;
    }

    int getOffset(const llvm::Value *value) {
        auto it = offset_map.find(value);
        if (it != offset_map.end()) {
            return it->second;
        }
        return allocateTempSlot(value);
    }

    void getOffsetMap(const llvm::Function &func) {
        offset_map.clear();
        local_mem = 4;

        if (!func.arg_empty()) {
            const llvm::Argument &arg = *func.arg_begin();
            offset_map[&arg] = 8;
        }

        for (const llvm::BasicBlock &bb : func) {
            for (const llvm::Instruction &inst : bb) {
                if (llvm::isa<llvm::AllocaInst>(inst)) {
                    local_mem += 4;
                    offset_map[&inst] = -local_mem;
                    continue;
                }

                if (const auto *store = llvm::dyn_cast<llvm::StoreInst>(&inst)) {
                    const llvm::Value *src = store->getValueOperand();
                    const llvm::Value *dst = store->getPointerOperand();

                    if (!func.arg_empty() && src == &*func.arg_begin()) {
                        offset_map[dst] = getOffset(src);
                    } else if (!isConstantInt(src)) {
                        offset_map[src] = getOffset(dst);
                    }
                    continue;
                }

                if (const auto *load = llvm::dyn_cast<llvm::LoadInst>(&inst)) {
                    offset_map[&inst] = getOffset(load->getPointerOperand());
                    continue;
                }

                if (hasResult(&inst)) {
                    allocateTempSlot(&inst);
                }
            }
        }
    }

    void computeLiveness(const llvm::BasicBlock &bb) {
        inst_index.clear();
        live_range.clear();

        // First pass: record where each value is defined in this block.
        int idx = 0;
        for (const llvm::Instruction &inst : bb) {
            if (isIgnoredAlloca(&inst)) {
                continue;
            }
            inst_index[&inst] = idx;
            if (hasResult(&inst)) {
                live_range[&inst] = std::make_pair(idx, idx);
            }
            ++idx;
        }

        // Second pass: stretch each live range to its last use in the block.
        for (const llvm::Instruction &inst : bb) {
            auto it_index = inst_index.find(&inst);
            if (it_index == inst_index.end()) {
                continue;
            }
            int use_index = it_index->second;
            for (const llvm::Use &use : inst.operands()) {
                const llvm::Value *operand = use.get();
                if (!isValueDefinedInBlock(operand, &bb)) {
                    continue;
                }
                const auto *def = llvm::cast<llvm::Instruction>(operand);
                auto it_live = live_range.find(def);
                if (it_live != live_range.end()) {
                    it_live->second.second = std::max(it_live->second.second, use_index);
                }
            }
        }
    }

    bool rangesOverlap(const llvm::Instruction *a, const llvm::Instruction *b) const {
        auto ita = live_range.find(a);
        auto itb = live_range.find(b);
        if (ita == live_range.end() || itb == live_range.end()) {
            return false;
        }
        return std::max(ita->second.first, itb->second.first) <=
               std::min(ita->second.second, itb->second.second);
    }

    void releaseOperandIfDead(const llvm::Value *operand, const llvm::Instruction *at_inst,
                              std::unordered_set<int> *available) {
        const auto *def = llvm::dyn_cast<llvm::Instruction>(operand);
        if (!def) {
            return;
        }
        auto it_live = live_range.find(def);
        auto it_reg = reg_map.find(def);
        if (it_live == live_range.end() || it_reg == reg_map.end() || it_reg->second == REG_NONE) {
            return;
        }
        auto it_idx = inst_index.find(at_inst);
        if (it_idx == inst_index.end()) {
            return;
        }
        if (it_live->second.second == it_idx->second) {
            available->insert(it_reg->second);
        }
    }

    void releaseOperandsEndingAt(const llvm::Instruction *inst, std::unordered_set<int> *available,
                                 const llvm::Instruction *except = nullptr) {
        for (const llvm::Use &use : inst->operands()) {
            const auto *operand_inst = llvm::dyn_cast<llvm::Instruction>(use.get());
            if (operand_inst && operand_inst == except) {
                continue;
            }
            releaseOperandIfDead(use.get(), inst, available);
        }
    }

    const llvm::Instruction *findSpill(const llvm::Instruction *inst,
                                       const std::vector<const llvm::Instruction *> &sorted_list) {
        for (const llvm::Instruction *candidate : sorted_list) {
            if (!rangesOverlap(candidate, inst)) {
                continue;
            }
            auto it = reg_map.find(candidate);
            if (it != reg_map.end() && it->second != REG_NONE) {
                return candidate;
            }
        }
        return nullptr;
    }

    void allocateRegisters(const llvm::Function &func) {
        reg_map.clear();

        for (const llvm::BasicBlock &bb : func) {
            computeLiveness(bb);
            std::unordered_set<int> available = {REG_EBX, REG_ECX, REG_EDX};

            std::vector<const llvm::Instruction *> sorted_list;
            for (const auto &entry : live_range) {
                sorted_list.push_back(entry.first);
            }
            std::sort(sorted_list.begin(), sorted_list.end(),
                      [&](const llvm::Instruction *lhs, const llvm::Instruction *rhs) {
                          const auto &left = live_range[lhs];
                          const auto &right = live_range[rhs];
                          if (left.second != right.second) {
                              return left.second > right.second;
                          }
                          return lhs->getNumUses() < rhs->getNumUses();
                      });

            // Allocation is local to the block, so every block starts with a clean register set.
            for (const llvm::Instruction &inst : bb) {
                if (isIgnoredAlloca(&inst)) {
                    continue;
                }

                if (!hasResult(&inst)) {
                    releaseOperandsEndingAt(&inst, &available);
                    continue;
                }

                bool reused_first_operand_reg = false;
                if (isArithmeticOp(&inst)) {
                    const llvm::Value *first = inst.getOperand(0);
                    if (const auto *first_inst = llvm::dyn_cast<llvm::Instruction>(first)) {
                        auto it_reg = reg_map.find(first_inst);
                        auto it_live = live_range.find(first_inst);
                        if (it_reg != reg_map.end() && it_live != live_range.end() &&
                            it_reg->second != REG_NONE &&
                            it_live->second.second == inst_index[&inst]) {
                            reg_map[&inst] = it_reg->second;
                            reused_first_operand_reg = true;
                            releaseOperandsEndingAt(&inst, &available, first_inst);
                            continue;
                        }
                    }
                }

                if (!available.empty()) {
                    int chosen = *std::min_element(available.begin(), available.end());
                    available.erase(chosen);
                    reg_map[&inst] = chosen;
                    releaseOperandsEndingAt(&inst, &available);
                    continue;
                }

                const llvm::Instruction *spill = findSpill(&inst, sorted_list);
                if (!spill) {
                    reg_map[&inst] = REG_NONE;
                    releaseOperandsEndingAt(&inst, &available);
                    continue;
                }

                int inst_uses = static_cast<int>(inst.getNumUses());
                int spill_uses = static_cast<int>(spill->getNumUses());
                int inst_end = live_range[&inst].second;
                int spill_end = live_range[spill].second;

                // Keep the register on the value that looks more expensive to spill.
                if (spill_uses > inst_uses || inst_end > spill_end) {
                    reg_map[&inst] = REG_NONE;
                } else {
                    reg_map[&inst] = reg_map[spill];
                    reg_map[spill] = REG_NONE;
                }
                if (!reused_first_operand_reg) {
                    releaseOperandsEndingAt(&inst, &available);
                }
            }
        }
    }

    std::optional<int> assignedReg(const llvm::Value *value) const {
        const auto *inst = llvm::dyn_cast<llvm::Instruction>(value);
        if (!inst) {
            return std::nullopt;
        }
        auto it = reg_map.find(inst);
        if (it == reg_map.end() || it->second == REG_NONE) {
            return std::nullopt;
        }
        return it->second;
    }

    void emitMoveValueToReg(const llvm::Value *value, const std::string &dest_reg) {
        if (isConstantInt(value)) {
            emitLine("\tmovl $" + std::to_string(getConstantInt(value)) + ", " + dest_reg);
            return;
        }

        if (auto reg = assignedReg(value)) {
            std::string src_reg = regName(*reg);
            if (src_reg != dest_reg) {
                emitLine("\tmovl " + src_reg + ", " + dest_reg);
            }
            return;
        }

        emitLine("\tmovl " + std::to_string(getOffset(value)) + "(%ebp), " + dest_reg);
    }

    std::string operandForArithmeticRHS(const llvm::Value *value) {
        if (isConstantInt(value)) {
            return "$" + std::to_string(getConstantInt(value));
        }
        if (auto reg = assignedReg(value)) {
            return regName(*reg);
        }
        return std::to_string(getOffset(value)) + "(%ebp)";
    }

    void emitLoad(const llvm::LoadInst &inst) {
        auto reg = assignedReg(&inst);
        if (!reg) {
            return;
        }
        int offset = getOffset(inst.getPointerOperand());
        emitLine("\tmovl " + std::to_string(offset) + "(%ebp), " + regName(*reg));
    }

    void emitStore(const llvm::StoreInst &inst, const llvm::Function &func) {
        const llvm::Value *src = inst.getValueOperand();
        const llvm::Value *dst = inst.getPointerOperand();

        if (!func.arg_empty() && src == &*func.arg_begin()) {
            return;
        }

        int dst_offset = getOffset(dst);
        if (isConstantInt(src)) {
            emitLine("\tmovl $" + std::to_string(getConstantInt(src)) + ", " +
                     std::to_string(dst_offset) + "(%ebp)");
            return;
        }

        if (auto reg = assignedReg(src)) {
            emitLine("\tmovl " + std::string(regName(*reg)) + ", " + std::to_string(dst_offset) +
                     "(%ebp)");
            return;
        }

        emitLine("\tmovl " + std::to_string(getOffset(src)) + "(%ebp), %eax");
        emitLine("\tmovl %eax, " + std::to_string(dst_offset) + "(%ebp)");
    }

    void emitArithmetic(const llvm::Instruction &inst) {
        std::string dest_reg = assignedReg(&inst) ? regName(*assignedReg(&inst)) : "%eax";
        emitMoveValueToReg(inst.getOperand(0), dest_reg);
        emitLine("\t" + arithMnemonic(inst.getOpcode()) + " " + operandForArithmeticRHS(inst.getOperand(1)) +
                 ", " + dest_reg);

        if (!assignedReg(&inst)) {
            emitLine("\tmovl %eax, " + std::to_string(getOffset(&inst)) + "(%ebp)");
        }
    }

    void emitCompare(const llvm::ICmpInst &inst) {
        std::string dest_reg = assignedReg(&inst) ? regName(*assignedReg(&inst)) : "%eax";
        emitMoveValueToReg(inst.getOperand(0), dest_reg);
        emitLine("\tcmpl " + operandForArithmeticRHS(inst.getOperand(1)) + ", " + dest_reg);
    }

    void emitCall(const llvm::CallInst &inst) {
        emitLine("\tpushl %ecx");
        emitLine("\tpushl %edx");

        const llvm::Function *callee = inst.getCalledFunction();
        bool has_arg = inst.arg_size() == 1;
        if (has_arg) {
            const llvm::Value *arg = inst.getArgOperand(0);
            if (isConstantInt(arg)) {
                emitLine("\tpushl $" + std::to_string(getConstantInt(arg)));
            } else if (auto reg = assignedReg(arg)) {
                emitLine("\tpushl " + std::string(regName(*reg)));
            } else {
                emitLine("\tpushl " + std::to_string(getOffset(arg)) + "(%ebp)");
            }
        }

        emitLine("\tcall " + callee->getName().str());

        if (has_arg) {
            emitLine("\taddl $4, %esp");
        }
        emitLine("\tpopl %edx");
        emitLine("\tpopl %ecx");

        if (!inst.getType()->isVoidTy()) {
            if (auto reg = assignedReg(&inst)) {
                emitLine("\tmovl %eax, " + std::string(regName(*reg)));
            } else {
                emitLine("\tmovl %eax, " + std::to_string(getOffset(&inst)) + "(%ebp)");
            }
        }
    }

    void emitBranch(const llvm::BranchInst &inst) {
        if (inst.isUnconditional()) {
            emitLine("\tjmp " + bb_labels[inst.getSuccessor(0)]);
            return;
        }

        const auto *cmp = llvm::dyn_cast<llvm::ICmpInst>(inst.getCondition());
        if (!cmp) {
            return;
        }
        emitLine("\t" + jumpMnemonic(cmp->getPredicate()) + " " + bb_labels[inst.getSuccessor(0)]);
        emitLine("\tjmp " + bb_labels[inst.getSuccessor(1)]);
    }

    void emitReturn(const llvm::ReturnInst &inst) {
        const llvm::Value *ret = inst.getReturnValue();
        if (ret) {
            if (isConstantInt(ret)) {
                emitLine("\tmovl $" + std::to_string(getConstantInt(ret)) + ", %eax");
            } else if (auto reg = assignedReg(ret)) {
                if (*reg != REG_NONE && std::string(regName(*reg)) != "%eax") {
                    emitLine("\tmovl " + std::string(regName(*reg)) + ", %eax");
                }
            } else {
                emitLine("\tmovl " + std::to_string(getOffset(ret)) + "(%ebp), %eax");
            }
        }

        emitLine("\tpopl %ebx");
        printFunctionEnd();
    }

    void emitInstruction(const llvm::Instruction &inst, const llvm::Function &func) {
        if (llvm::isa<llvm::AllocaInst>(inst)) {
            return;
        }
        if (const auto *load = llvm::dyn_cast<llvm::LoadInst>(&inst)) {
            emitLoad(*load);
            return;
        }
        if (const auto *store = llvm::dyn_cast<llvm::StoreInst>(&inst)) {
            emitStore(*store, func);
            return;
        }
        if (const auto *call = llvm::dyn_cast<llvm::CallInst>(&inst)) {
            emitCall(*call);
            return;
        }
        if (const auto *branch = llvm::dyn_cast<llvm::BranchInst>(&inst)) {
            emitBranch(*branch);
            return;
        }
        if (const auto *ret = llvm::dyn_cast<llvm::ReturnInst>(&inst)) {
            emitReturn(*ret);
            return;
        }
        if (isArithmeticOp(&inst)) {
            emitArithmetic(inst);
            return;
        }
        if (const auto *icmp = llvm::dyn_cast<llvm::ICmpInst>(&inst)) {
            emitCompare(*icmp);
            return;
        }
    }

    void emitFunction(const llvm::Function &func) {
        createBBLabels(func);
        getOffsetMap(func);
        allocateRegisters(func);

        printDirectives(func);
        emitLine("\tpushl %ebp");
        emitLine("\tmovl %esp, %ebp");
        emitLine("\tsubl $" + std::to_string(local_mem) + ", %esp");
        emitLine("\tpushl %ebx");

        for (const llvm::BasicBlock &bb : func) {
            emitLine(bb_labels[&bb] + ":");
            for (const llvm::Instruction &inst : bb) {
                emitInstruction(inst, func);
            }
        }

        emitLine("");
    }
};

bool emitModule(const llvm::Module &module, std::ostream &out) {
    Backend backend(out);
    bool emitted_any = false;
    for (const llvm::Function &func : module) {
        if (func.isDeclaration()) {
            continue;
        }
        backend.emitFunction(func);
        emitted_any = true;
    }
    return emitted_any;
}

}  // namespace

int main(int argc, char **argv) {
    llvm::InitLLVM init_llvm(argc, argv);

    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <input.ll> [output.s]\n";
        return 1;
    }

    llvm::LLVMContext context;
    llvm::SMDiagnostic error;
    std::unique_ptr<llvm::Module> module = llvm::parseIRFile(argv[1], error, context);
    if (!module) {
        error.print(argv[0], llvm::errs());
        return 2;
    }

    if (argc == 3) {
        std::ofstream out(argv[2]);
        if (!out) {
            std::cerr << "Failed to open output file '" << argv[2] << "'\n";
            return 3;
        }
        if (!emitModule(*module, out)) {
            return 4;
        }
        return 0;
    }

    if (!emitModule(*module, std::cout)) {
        return 4;
    }
    return 0;
}
