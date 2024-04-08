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
#include <set>

using namespace llvm;

bool runOnFunction(Function&);
bool runOnBasicBlock(BasicBlock&);

bool optBasicSR(Instruction&);
bool optAlgId(Instruction&);
bool optAdvSR(Instruction&);
bool optMultiInstr(Instruction&);


PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  bool modified = false;

  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter) {
    modified = modified | runOnFunction(*Fiter);
  }

  return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
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

bool runOnBasicBlock(BasicBlock &B) {
  bool globallyModified = false;
  std::set<Instruction*> toBeErased;

  for (auto &I : B) {
    // Be aware that the short-circuiting property of logical OR implies
    // that for each instruction, just the first matching optimization is executed.
    // Comment one of the following lines to disable the respective optimization
    bool locallyModified =
      optBasicSR(I)
        || optAlgId(I)
        || optAdvSR(I)
        || optMultiInstr(I);

    if (locallyModified) toBeErased.emplace(&I);
        
    globallyModified = globallyModified or locallyModified;
  }

  if (globallyModified) {
    for (auto &I : toBeErased) I->eraseFromParent();
  }

  errs() << (globallyModified ? "IR has been modified" : "Nothing has been modified") << "\n";

  return globallyModified;
}

/**
 * SYNOPSIS
 * An optimization pass that acts on a single mul or udiv
 * asm instruction to apply strength reduction.
 * 
 * DETAILS
 * Multiplications and divisions by a power of 2 are
 * turned into shift left of right. Takes into account
 * that mul is commutative, while div is not.
 * 
 * EXAMPLES
 * mul %0, 8 -> shl %0, 3
 * mul 8, %0 -> shl %0, 3
 * mul 8, 16 -> shl 16, 3
 * udiv %0, 8 -> lshr %0, 3
 * udiv 16, 8 -> lshr 16, 3
*/
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
      and not CI->isOne();
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

/**
 * SYNOPSIS
 * An optimization pass that acts on a single mul or udiv
 * asm instruction to apply algebraic semplification.
 * 
 * DETAILS
 * Removes muls that have "1" as a operand, by replacing
 * them with the other operand.
 * Removes adds that have "0" as a operand, by replacing
 * thetm with the other operand.
 * 
 * EXAMPLES
 * mul %0, 1 => %0
 * mul 1, %0 => %0
 * add %0, 0 => %0
 * add 0, %0 => %0
*/
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

/**
 * SYNOPSIS
 * An optimization pass that acts on a single mul asm
 * instruction to apply advanced strength reduction.
 * 
 * DETAILS
 * Multiplications that have a operand which is almost
 * (1 step - add or sub - far) a power of 2, are turned
 * into a shift left plus a sub or an add.
 * 
 * EXAMPLES
 * mul %0, 9 -> %1 = shl %0, 3; add %1, %0
 * mul 15, %0 -> %1 = shl %0, 4; sub %1, %0
 * mul 31, 8 -> %1 = shl 8, 5; sub %1, 8
*/
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
      and APInt(CI->getValue() + 1).isPowerOf2()
      and not CI->isZero();
  };

  // Check if op is a constant integer and can be turned into
  // a power of 2 with 1 subtraction
  // Warning: this lambda has a side effect
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

  if (CI->getValue().isOne()) return false;
    
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

/**
 * SYNOPSIS
 * Multi-instruction optimization pass that acts on patterns
 * in which an instruction nullifies the previous one. 
 * 
 * DETAILS
 * Able to manage also instructions arranged in a "sandwitch"
 * fashion - i.e. if the nullification is not immediately
 * after the nullified operation.
 * 
 * EXAMPLES
 * %2 = sub i32 %0, 10; [STUFF;] %4 = add i32 %2, 10
 * => %2 = sub i32 %0, 10; [STUFF;] %4 = %0
 * 
 * %2 = add i32 %0, 20; [STUFF;] %4 = sub i32 %2, 20
 * => %2 = add i32 %0, 20; [STUFF;] %4 = %0
*/
bool optMultiInstr(Instruction &I) {
  auto OpCode = I.getOpcode();

  if (OpCode != Instruction::Add and OpCode != Instruction::Sub) return false;

  using OptimizableInstr = std::optional<std::pair<Value*, ConstantInt*>>;
  auto tryGetOperands = [](const Instruction &I) -> OptimizableInstr {
    Value *Op1 = I.getOperand(0);
    Value *Op2 = I.getOperand(1);
    ConstantInt *CI = nullptr;

    const bool isCommutative = I.getOpcode() == Instruction::Add;
    if (isCommutative and (CI = dyn_cast<ConstantInt>(Op1))) std::swap(Op1, Op2);
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

  // If the previous instruction has not the desired structure, exit
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

  errs() << "Triggered multi-instruction optimization\n";

  I.replaceAllUsesWith(PrevInstrOperands->first);
  
  return true;
}