//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include <map>
#include <optional>

using namespace llvm;

bool optBasicSR(Instruction &I) {
    auto OpCode = I.getOpcode();


    if (OpCode != Instruction::Mul and OpCode != Instruction::UDiv) return false;

    Value *Op1 = I.getOperand(0);
    Value *Op2 = I.getOperand(1);
    ConstantInt *CI = nullptr;

    // Warning: this lambda has a side effect
    auto isConstPowOf2 = [&CI](auto &op) {
        return (CI = dyn_cast<ConstantInt>(op))
            and CI->getValue().isPowerOf2()
            and CI->getValue() != 1;
    };

    if (OpCode == Instruction::Mul) {
        if (isConstPowOf2(Op1)) std::swap(Op1, Op2);
        if (not isConstPowOf2(Op2)) return false;
    }
    else if (OpCode == Instruction::UDiv) {
        if (not isConstPowOf2(Op2)) return false;
    }

    // If we reach this point, Op2 is always a constant integer, power of 2
    // The following code in hence invariant wrt operands order

    errs() << "Triggered mul pow of 2 to shift\n";

    // Shift amount calculation using the log2 of the constant
    unsigned ShiftAmount = CI->getValue().logBase2();

    // Create a new shift instruction
    Instruction::BinaryOps ShiftType = 
        OpCode == Instruction::Mul ? Instruction::Shl : Instruction::LShr;
    Instruction *ShiftInst = BinaryOperator::Create(
        ShiftType,
        Op1, ConstantInt::get(CI->getType(), ShiftAmount)
    );
    
    ShiftInst->insertAfter(&I);
    I.replaceAllUsesWith(ShiftInst);

    return true;
}

bool optAlgId(Instruction &I) {
    auto OpCode = I.getOpcode();

    if (OpCode != Instruction::Add and OpCode != Instruction::Mul) return false;

    Value *Op1 = I.getOperand(0);
    Value *Op2 = I.getOperand(1);
    ConstantInt *CI = nullptr;

    // Is the operator a neutral constant of the operation?
    // CI is set to the constant neutral operand after this function call
    // Warning: this lambda has a side effect
    std::function<bool(Value*)> isNeutralConstant;

    if (OpCode == Instruction::Mul) {
        isNeutralConstant = [&CI] (Value *op) {
            return (CI = dyn_cast<ConstantInt>(op))
                and CI->isOne();
        };
    }
    
    if (OpCode == Instruction::Add) {
        isNeutralConstant = [&CI] (Value *op) {
            return (CI = dyn_cast<ConstantInt>(op))
                and CI->isZero();
        };
    }

    if (isNeutralConstant(Op1)) std::swap(Op1, Op2);
    if (not isNeutralConstant(Op2)) return false;

    // If we reach this point, Op2 is always a constant integer, neutral for the operation
    // The following code in hence invariant wrt operands order

    errs() << "Triggered algebraic ID\n";

    I.replaceAllUsesWith(Op1);

    return true;
}

bool optAdvSR(Instruction &I) {
    auto OpCode = I.getOpcode();

    if (OpCode != Instruction::Mul) return false;

    Value *Op1 = I.getOperand(0);
    Value *Op2 = I.getOperand(1);
    ConstantInt *CI;

    // Check if op is a constant integer and can be turned into
    // a power of 2 with 1 sum
    // Warning: this lambda has a side effect
    auto isPow2MinusOne = [&CI](Value *op) {
        return (CI = dyn_cast<ConstantInt>(op))
            and APInt(CI->getValue() + 1).isPowerOf2();
    };

    // Check if op is a constant integer and can be turned into
    // a power of 2 with 1 subtraction
    auto isPow2PlusOne = [&CI](Value *op) {
        return (CI = dyn_cast<ConstantInt>(op))
            and APInt(CI->getValue() - 1).isPowerOf2();
    };

    auto isAlmostPow2 = [&isPow2PlusOne, &isPow2MinusOne](Value *op) {
        return isPow2PlusOne(op) or isPow2MinusOne(op);
    };

    if (isAlmostPow2(Op1)) std::swap(Op1, Op2);
    if (not isAlmostPow2(Op2)) return false;

    // If we reach this point, Op2 is always a constant integer, distant 1 Operation from
    // a power of 2. The following code in hence invariant wrt operands order

    errs() << "Triggered advanced strength reduction\n";

    // Adjustment Instruction Type
    Instruction::BinaryOps AdjInstType =
        isPow2PlusOne(Op2) ? Instruction::Add : Instruction::Sub;

    APInt Pow2Val =
        isPow2PlusOne(Op2) ? (CI->getValue() - 1) : (CI->getValue() + 1);
    
    unsigned ShiftVal =  Pow2Val.logBase2();

    Instruction *ShftInst = BinaryOperator::Create(
        Instruction::Shl,
        Op1, ConstantInt::get(CI->getType(), ShiftVal)
    );
    Instruction *AdjInst = BinaryOperator::Create(
        AdjInstType,
        ShftInst, Op1
    );

    ShftInst->insertAfter(&I);
    AdjInst->insertAfter(ShftInst);
    
    I.replaceAllUsesWith(AdjInst);

    return true;
}

bool optMultiInstr(Instruction &I) {
    auto OpCode = I.getOpcode();

    if (OpCode != Instruction::Add and OpCode != Instruction::Sub) return false;

    using OptimizableInstr = std::optional<std::pair<Value*, ConstantInt*>>;
    auto tryGetOperands = [](const Instruction &I) -> OptimizableInstr {
        Value *Op1 = I.getOperand(0);
        Value *Op2 = I.getOperand(1);
        ConstantInt *CI = nullptr;

        if ((CI = dyn_cast<ConstantInt>(Op1))) std::swap(Op1, Op2);
        if (not (CI = dyn_cast<ConstantInt>(Op2))) return std::nullopt;

        // Invariant code wrt operands order
        return OptimizableInstr{{Op1, CI}};
    };

    // If this instruction has not the desired structure, exit
    OptimizableInstr ThisInstrOperands = tryGetOperands(I);
    if (not ThisInstrOperands) return false;
    
    // Or if the non constant operand is not the result of a
    // previous instruction (eg. a constant), exit
    Instruction *PrevInstr = dyn_cast<Instruction>(ThisInstrOperands->first);
    if (not PrevInstr) return false;

    // If thre previous instruction has not the desired structure, exit
    OptimizableInstr PrevInstrOperands = tryGetOperands(*PrevInstr);
    if (not PrevInstrOperands) return false;

    // Map of inverse operations
    const std::map<Instruction::BinaryOps, Instruction::BinaryOps> InverseOperators = {
        {Instruction::Add, Instruction::Sub},
        {Instruction::Sub, Instruction::Add}
    };

    bool areInverseOperations = 
        InverseOperators.at(static_cast<Instruction::BinaryOps>(OpCode)) == PrevInstr->getOpcode();
    if (not areInverseOperations) return false;

    bool haveSameConstant =
        ThisInstrOperands->second->getValue() == PrevInstrOperands->second->getValue();
    if (not haveSameConstant) return false;

    I.replaceAllUsesWith(PrevInstrOperands->first);
    return true;
}

void eraseInstructions(BasicBlock &B) {
    std::vector<Instruction*> toErase;

    for (auto &I : B) {
        if (optBasicSR(I) || optAlgId(I) || optAdvSR(I) || optMultiInstr(I)) {
            toErase.push_back(&I);
        }
    }

    for (auto *I : toErase) {
        I->eraseFromParent();
    }
}

bool runOnBasicBlock(BasicBlock &B) {
    bool modified = false;
    for (auto &I : B) {
        modified = modified | optBasicSR(I) | optAlgId(I) | optAdvSR(I) | optMultiInstr(I);
    }

    if (modified) {
        eraseInstructions(B);
        errs() << "IR has been modified\n";
    } else {
        errs() << "Nothing has been modified\n";
    }

    return modified;
}

bool runOnFunction(Function &F) {
    errs() << "\nRunning on function: " << F.getName() << "\n";
    bool Transformed = false;

    for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
        if (runOnBasicBlock(*Iter)) {
            Transformed = true;
        }
    }

    return Transformed;
}


PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
    bool modified = false;

    for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter) {
        modified = modified | runOnFunction(*Fiter);
    }

    return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
