#include "llvm/Transforms/Utils/LoopFusion.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
using namespace llvm;


bool isAdjacent(Loop *prevLoop, Loop *nextLoop){
    auto prevExitBB = prevLoop->getBlocks().back();

    // if the loop is guarded the entry block to check 
    // is the block which contains the guard branch
    auto nextPreheader = nextLoop->isGuarded() ? 
        nextLoop->getLoopGuardBranch()->getParent() : nextLoop->getLoopPreheader();

    return prevExitBB == nextPreheader;
}

bool isControlFlowEquivalent(Function &F, FunctionAnalysisManager &AM, Loop *prevLoop, Loop *nextLoop){
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    
    auto prevHeader = prevLoop->getHeader();
    auto nextHeader = nextLoop->getHeader();

    // return true iff the prevLoop dominates the nextLoop
    // and iff the nextLoop postdominates the prevLoop
    return DT.dominates(prevHeader, nextHeader) and PDT.dominates(nextHeader, prevHeader);
}

void optimize(Loop *prevLoop, Loop *nextLoop){
    
    errs() << prevLoop->getName() << " and " << nextLoop->getName() << 
        "are adjacent and control flow equivalent\n";
    
}


PreservedAnalyses LoopFusion::run(Function &F,FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    
    Loop *prevLoop = nullptr;
    bool optimizable = false;
    for (auto *L : LI){
        if(prevLoop)
            optimizable = isAdjacent(prevLoop, L) 
                and isControlFlowEquivalent(F, AM, prevLoop, L);
            
            if(optimizable) optimize(prevLoop, L);


        prevLoop = L;
    }

    return PreservedAnalyses::all();
}


