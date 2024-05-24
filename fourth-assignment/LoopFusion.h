#pragma once
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"

namespace llvm {
    class LoopFusion : public PassInfoMixin<LoopFusion> {
    public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
    };
} // namespace llvm
