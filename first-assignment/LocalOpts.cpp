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
                    errs() << "Triggered algebraic ID\n";
                    AddInst->replaceAllUsesWith(Op2);
                    AddInst->eraseFromParent();
                    Modified = true;
                }
                else if ((CI = dyn_cast<ConstantInt>(Op2)) && CI->isZero()) {
                    errs() << "Triggered algebraic ID\n";
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
                    // swap operands in order to have the power of 2 as the first operand (better for optimization)
                    std::swap(Op1, Op2);
                } 
                else {
                    continue;
                }

                errs() << "Triggered mul pow of 2 to shift\n";

                // shift amount calculation using the log2 of the constant
                unsigned ShiftAmount = CI->getValue().logBase2();

                // create a new shift instruction
                Instruction *ShlInst = BinaryOperator::CreateShl(Op2, ConstantInt::get(CI->getType(), ShiftAmount));
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

                // check for mul by 15 (2^4 - 1) -> x*15 or 15*x
                if (((CI == dyn_cast<ConstantInt>(Op1)) && CI->getValue() == 15) ||
                    ((CI == dyn_cast<ConstantInt>(Op2)) && CI->getValue() == 15)) {
                        errs() << "Triggered mul advanced strength reduction\n";
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
                    errs() << "Triggered div advanced strength reduction\n";
                    // implement x/8 as x>>3
                    Instruction *ShrInst = BinaryOperator::CreateLShr(Op1, ConstantInt::get(CI->getType(), 3), "", DivInst);
                    DivInst->replaceAllUsesWith(ShrInst);
                    DivInst->eraseFromParent();
                    Modified = true;
                }
            }
        }

        /* THIRD ASSIGNMENT -> MULTI-INSTRUCTION OPTIMIZATION */
        if (auto *AddInst = dyn_cast<BinaryOperator>(&I)) {
            // check for addition with 1
            if (AddInst->getOpcode() == Instruction::Add) {
                Value *Op1 = AddInst->getOperand(0);
                ConstantInt *CI = dyn_cast<ConstantInt>(AddInst->getOperand(1));
                if (CI && CI->isOne()) {
                    // look for the next instruction to be a substraction
                    Instruction *NextI = AddInst->getNextNode();
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
                                    SubInst->eraseFromParent();
                                    AddInst->eraseFromParent();
                                    Modified = true;
                                    // break the loop immediately after the erase
                                    return Modified;
                                }
                            }
                        }
                    }
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


PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
    if (runOnFunction(*Fiter))
      return PreservedAnalyses::none();
  
  return PreservedAnalyses::all();
}
