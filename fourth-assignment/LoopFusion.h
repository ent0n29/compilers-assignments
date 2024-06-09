#pragma once
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;
struct LoopFusion : PassInfoMixin<LoopFusion> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};