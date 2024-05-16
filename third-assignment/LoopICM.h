#include "llvm/IR/PassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"

using namespace llvm;

struct LoopICM : public PassInfoMixin<LoopICM> {
  PreservedAnalyses run(Loop&, LoopAnalysisManager&, LoopStandardAnalysisResults&, LPMUpdater&);
};
