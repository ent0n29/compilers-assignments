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

bool runOnBasicBlock(BasicBlock &B) {
    bool Modified = false;
    for (auto &I : B) {

        /* FIRST ASSIGNMENT -> ALGEBRAIC IDENTITY */
        if (auto *AddInst = dyn_cast<BinaryOperator>(&I)) {
            if (AddInst->getOpcode() == Instruction::Add) {
                Value *Op1 = AddInst->getOperand(0);
                Value *Op2 = AddInst->getOperand(1);
                ConstantInt *CI = nullptr;
                if ((CI = dyn_cast<ConstantInt>(Op1)) && CI->isZero()) {
                    AddInst->replaceAllUsesWith(Op2);
                    AddInst->eraseFromParent();
                    Modified = true;
                }
                else if ((CI = dyn_cast<ConstantInt>(Op2)) && CI->isZero()) {
                    AddInst->replaceAllUsesWith(Op1);
                    AddInst->eraseFromParent();
                    Modified = true;
                }
            }   
        }

        /* STRENGTH REDUCTION -> SHIFT <<2 FOR POWER OF 2 MULTIPLICATION*/
        if (auto *MulInst = dyn_cast<BinaryOperator>(&I)) {
            if (MulInst->getOpcode() == Instruction::Mul) {
                Value *Op1 = MulInst->getOperand(0);
                Value *Op2 = MulInst->getOperand(1);
                ConstantInt *CI = nullptr;

                // check if operand is a power of 2
                if ((CI = dyn_cast<ConstantInt>(Op1)) && CI->getValue().isPowerOf2()) {
                    // do nothing
                } 
                else if ((CI = dyn_cast<ConstantInt>(Op2)) && CI->getValue().isPowerOf2()) {
                    // swap operands in order to have the power of 2 as the second operand (better for optimization)
                    std::swap(Op1, Op2);
                } 
                else {
                    continue;
                }

                // shift amount calculation using the log2 of the constant
                unsigned ShiftAmount = CI->getValue().logBase2();

                // create a new shift instruction
                Instruction *ShlInst = BinaryOperator::CreateShl(Op1, ConstantInt::get(CI->getType(), ShiftAmount));
                ShlInst->insertAfter(MulInst);
                MulInst->replaceAllUsesWith(ShlInst);
                ShlInst->setDebugLoc(MulInst->getDebugLoc());

                // erase the original multiplication instruction
                MulInst->eraseFromParent();

                return true;
            }
        }

        /* SECOND ASSIGNMENT -> ADVANCED STRENGTH REDUCTION */
        if (auto *MultInst = dyn_cast<BinaryOperator>(&I)) {
            if (MultInst->getOpcode() == Instruction::Mul) {
                Value *Op1 = MultInst->getOperand(0);
                Value *Op2 = MultInst->getOperand(1);
                ConstantInt *CI = nullptr;

                // check for mul by 15 (2^4 - 1)
                if (((CI == dyn_cast<ConstantInt>(Op1)) && CI->getValue() == 15) ||
                    ((CI == dyn_cast<ConstantInt>(Op2)) && CI->getValue() == 15)) {
                        // change x*15 to (x<<4) - x
                        Value *Shifted = BinaryOperator::CreateShl(Op1, ConstantInt::get(CI->getType(), 4), "", MultInst);
                        Instruction *Sub = BinaryOperator::CreateSub(Shifted, Op1, "", MultInst);
                        MultInst->replaceAllUsesWith(Sub);
                        MultInst->eraseFromParent();
                        Modified = true;
                }  
            }
        }
        else if (auto *DivInst = dyn_cast<BinaryOperator>(&I)) {
            if (DivInst->getOpcode() == Instruction::UDiv) {
                Value *Op1 = DivInst->getOperand(0);
                ConstantInt *CI = nullptr;

                // check for division by 8 (2^3)
                if ((CI = dyn_cast<ConstantInt>(DivInst->getOperand(1))) && CI->getValue() == 8) {
                    // implement x/8 as x>>3
                    Instruction *ShrInst = BinaryOperator::CreateLShr(Op1, ConstantInt::get(CI->getType(), 3), "", DivInst);
                    DivInst->replaceAllUsesWith(ShrInst);
                    DivInst->eraseFromParent();
                    Modified = true;
                }
            }
        }
    }
    return Modified;
}




bool runOnFunction(Function &F) {
  bool Transformed = false;

  for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (runOnBasicBlock(*Iter)) {
      Transformed = true;
    }
  }

  return Transformed;
}


PreservedAnalyses LocalOpts::run(Module &M,
                                      ModuleAnalysisManager &AM) {
  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
    if (runOnFunction(*Fiter))
      return PreservedAnalyses::none();
  
  return PreservedAnalyses::all();
}

