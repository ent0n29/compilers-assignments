#include "llvm/Transforms/Utils/LoopFusion.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
using namespace llvm;


bool isAdjacent(Loop *prevLoop, Loop *nextLoop){

    // if the loop has multiple exits the function returns null 
    auto prevExitBB = prevLoop->getExitBlock(); 

    // loop with multiple exits can't be fused
    if (not prevExitBB) return false; 

    // if the loop is guarded the entry block to check 
    // is the block which contains the guard branch
    auto nextPreheader = nextLoop->isGuarded() ? 
        nextLoop->getLoopGuardBranch()->getParent() : nextLoop->getLoopPreheader();

    prevExitBB->print(errs());
    nextPreheader->print(errs());
    errs() << "Result: " << (prevExitBB == nextPreheader);
    return prevExitBB == nextPreheader;
}

bool isControlFlowEquivalent(Function &F, FunctionAnalysisManager &AM, Loop *prevLoop, Loop *nextLoop){
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    
    auto prevPreheader = prevLoop->getLoopPreheader();
    auto nextPreheader = nextLoop->getLoopPreheader();

    // return true iff the prevLoop dominates the nextLoop
    // and iff the nextLoop postdominates the prevLoop
    return DT.dominates(prevPreheader, nextPreheader) and PDT.dominates(nextPreheader, prevPreheader);
}

bool hasSameTripCount(Function &F, FunctionAnalysisManager &AM, Loop *prevLoop, Loop *nextLoop){
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);

    // TO DO: with variables as UB it returns 0
    auto prevExitCount = SE.getSmallConstantMaxTripCount(prevLoop);
    auto nextExitCount = SE.getSmallConstantMaxTripCount(nextLoop);

    errs() << "\nprevLoop: " << prevExitCount; 
    errs() << "\nnextLoop: " << nextExitCount;
    
    //if(isa<SCEVCouldNotCompute>(prevExitCount) or isa<SCEVCouldNotCompute>(nextExitCount)) return false;
    
    return prevExitCount == nextExitCount;
}

Loop * optimize(Loop *prevLoop, Loop *nextLoop){

    prevLoop->getHeader()->print(errs());
    errs() << "\n and \n ";
    nextLoop->getHeader()->print(errs());
    errs() << "are adjacent and control flow equivalent\n"; 

    // when optmized the function must return the new fused loop 
    // atm with no optimization returns the current loop 
    return prevLoop;
}


PreservedAnalyses LoopFusion::run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    
    errs() << "Started Loop Fusion\n";

    Loop *prevLoop = nullptr;
    bool optimizable = false;

    for (auto lit = LI.rbegin(); lit != LI.rend(); ++lit){
        if(prevLoop)
            optimizable = isAdjacent(prevLoop, *lit)   
                and isControlFlowEquivalent(F, AM, prevLoop, *lit) 
                and hasSameTripCount(F, AM, prevLoop, *lit);
        
        prevLoop = optimizable ? optimize(*lit, prevLoop) : *lit;

    }

    return PreservedAnalyses::all();
}

