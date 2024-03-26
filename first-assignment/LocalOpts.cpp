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

using namespace llvm;

bool optBasicSR(Instruction &I) {
    if (I.getOpcode() != Instruction::Mul) return false;
    auto *MulInst = &I;

    Value *Op1 = MulInst->getOperand(0);
    Value *Op2 = MulInst->getOperand(1);
    ConstantInt *CI = nullptr;

    auto isConstPowOf2 = [&CI](auto &op) mutable {
        return (CI = dyn_cast<ConstantInt>(op))
            and CI->getValue().isPowerOf2()
            and CI->getValue() != 1;
    };

    if (isConstPowOf2(Op1)) std::swap(Op1, Op2);
    if (not isConstPowOf2(Op2)) return false;

    // If we reach this point, Op2 is always a constant integer, power of 2
    // The following code in hence invariant wrt operands order

    errs() << "Triggered mul pow of 2 to shift\n";

    // Shift amount calculation using the log2 of the constant
    unsigned ShiftAmount = CI->getValue().logBase2();

    // Create a new shift instruction
    Instruction *ShlInst = BinaryOperator::CreateShl(Op1, ConstantInt::get(CI->getType(), ShiftAmount));
    
    ShlInst->insertAfter(MulInst);
    MulInst->replaceAllUsesWith(ShlInst);
    ShlInst->setDebugLoc(MulInst->getDebugLoc());

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
    std::function<bool(Value*)> isNeutralConstant;

    if (OpCode == Instruction::Mul) {
        isNeutralConstant = [&CI] (Value *op) mutable {
            return (CI = dyn_cast<ConstantInt>(op))
                and CI->isOne();
        };
    }
    
    if (OpCode == Instruction::Add) {
        isNeutralConstant = [&CI] (Value *op) mutable {
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
    if (I.getOpcode() != Instruction::Mul and I.getOpcode() != Instruction::UDiv) return false;

    if (I.getOpcode() == Instruction::Mul) {
        BinaryOperator *MultInst = dyn_cast<BinaryOperator>(&I);

        Value *Op1 = MultInst->getOperand(0);
        Value *Op2 = MultInst->getOperand(1);
        ConstantInt *CI = nullptr;

        // check for mul by 15 (2^4 - 1) -> x*15 or 15*x
        if (((CI == dyn_cast<ConstantInt>(Op1)) && CI->getValue() == 15) ||
            ((CI == dyn_cast<ConstantInt>(Op2)) && CI->getValue() == 15)) {
                errs() << "Triggered mul advanced strength reduction\n";
                // change x*15 to (x<<4) - x
                Value *Shifted = BinaryOperator::CreateShl(Op1, ConstantInt::get(CI->getType(), 4));
                Instruction *Sub = BinaryOperator::CreateSub(Shifted, Op1);
                MultInst->replaceAllUsesWith(Sub);
                // MultInst->eraseFromParent();
                return true;
        }
    }
    if (I.getOpcode() == Instruction::UDiv) {
        BinaryOperator *DivInst = dyn_cast<BinaryOperator>(&I);

        Value *Op1 = DivInst->getOperand(0);
        ConstantInt *CI = nullptr;

        // check for division by 8 (2^3)
        if ((CI = dyn_cast<ConstantInt>(DivInst->getOperand(1))) && CI->getValue() == 8) {
            errs() << "Triggered div advanced strength reduction\n";
            // implement x/8 as x>>3
            Instruction *ShrInst = BinaryOperator::CreateLShr(Op1, ConstantInt::get(CI->getType(), 3));
            DivInst->replaceAllUsesWith(ShrInst);
            // DivInst->eraseFromParent();
            return true;
        }
    }

    return false;
}

bool optMultiInstr(Instruction &I) {
    if (auto *AddInst = dyn_cast<BinaryOperator>(&I)) {
        // check for addition with 1
        if (AddInst->getOpcode() == Instruction::Add) {
            Value *Op1 = AddInst->getOperand(0);
            ConstantInt *CI = dyn_cast<ConstantInt>(AddInst->getOperand(1));
            if (CI && CI->isOne()) {
                // look for the next instruction to be a substraction
                Instruction *NextI = I.getNextNode();
                if (NextI){
                    if (auto *SubInst = dyn_cast<BinaryOperator>(NextI)) {
                        if (SubInst->getOpcode() == Instruction::Sub) {
                            Value *SubOp1 = SubInst->getOperand(0);
                            ConstantInt *SubCI = dyn_cast<ConstantInt>(SubInst->getOperand(1));
                            // check if the sub is undoing the addition (the pattern)
                            if (SubOp1 == AddInst && SubCI && SubCI->isOne()) {
                                errs() << "Triggered advanced multi-instr optimization\n";

                                // replace uses of the sub with the original operand of the add
                                SubInst->replaceAllUsesWith(Op1);
                                // erase sub instruction
                                // SubInst->eraseFromParent();
                                // AddInst->eraseFromParent(); // core dumps here
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return false;
}

bool runOnBasicBlock(BasicBlock &B) {
    bool modified = false;
    for (auto &I : B) {
        modified =
            optBasicSR(I)
            | optAlgId(I)
            // | optAdvSR(I)
            // | optMultiInstr(I)
            | modified;
    }

    errs() << (modified ? "IR has been modified" : "Nothing has been modified") << "\n";

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
