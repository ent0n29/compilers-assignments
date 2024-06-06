#include "llvm/Transforms/Utils/LoopFusion.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/DependenceAnalysis.h"
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

    auto prevBackedgeCount = SE.getBackedgeTakenCount(prevLoop);
    auto nextBackedgeCount = SE.getBackedgeTakenCount(nextLoop);

    errs() << "\nprevLoop trip count: "; 
    prevBackedgeCount->print(errs()); 
    errs() << "\nnextLoop trip count: ";
    nextBackedgeCount->print(errs());
    errs() << "\n";
    
    // if one of the results is not computable exit
    if(isa<SCEVCouldNotCompute>(prevBackedgeCount) or isa<SCEVCouldNotCompute>(nextBackedgeCount)) return false;
    
    // returns true if SCEV objects are equal, false otherwise
    return SE.isKnownPredicate(ICmpInst::ICMP_EQ, prevBackedgeCount, nextBackedgeCount);
}

/**
 * Checks for any R-W or W-R dependency between the pair of loops passed to the function.
 * As in the original LLVM's loop-fusion pass, there is no real distinction between negative/positive/neutral dependencies.
 * Any form of dependcy is prohibited in order to activate the optimization. 
*/
bool hasNegativeDistanceDependencies(Function &F, FunctionAnalysisManager &AM, Loop *prevLoop, Loop *nextLoop){
    errs() << "Checking negative dependencies\n";
    
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);
    SmallVector<Instruction*> prevStores, prevLoads, nextLoads, nextStores;

    auto areDependent = [&DI] (Instruction *x, Instruction *y) {
        auto dep = DI.depends(x, y, true);
        return dep and not dep->isConfused();
    };
    
    for (auto *bb : prevLoop->blocks()) {
        for (auto &i : *bb) {
            if (i.mayWriteToMemory()) prevStores.push_back(&i);
            if (i.mayReadFromMemory()) prevLoads.push_back(&i);
        }
    }

    for (auto *bb : nextLoop->blocks()) {
        for (auto &i : *bb) {
            if (i.mayWriteToMemory()) nextStores.push_back(&i);
            if (i.mayReadFromMemory()) nextLoads.push_back(&i);        }
    }

    // Store-load, Load-store access patterns can cause problems
    for (auto &prevI : prevStores) {
        for (auto &nextI : nextLoads) if (areDependent(prevI, nextI)) return false;
    }

    for (auto &prevI : prevLoads) {
        for (auto &nextI : nextStores) if (areDependent(prevI, nextI)) return false;
    }

    return true;
}

Loop * optimize(Function &F, FunctionAnalysisManager &AM, Loop *prevLoop, Loop *nextLoop){
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  
    errs() << "Optimizable\n"; 
    
    auto nextIV = nextLoop->getInductionVariable(SE);

/*   for(auto userIter = nextIV->user_begin(); userIter != nextIV->user_end(); ++userIter){
        User *user = *userIter;

        user->print(errs());
        errs() << "\n";
    }*/


    // when optmized the function must return the new fused loop 
    // atm with no optimization returns the current loop 
    return prevLoop;
}


PreservedAnalyses LoopFusion::run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    
    errs() << "Started Loop Fusion\n";

    Loop *prevLoop = nullptr;
    bool optimizable = false;
    int num = 0;

    for (auto lit = LI.rbegin(); lit != LI.rend(); ++lit){
        errs() << "Analysing loop "<< num++ << "\n";

        if(prevLoop)
            optimizable = isAdjacent(prevLoop, *lit)   
                and isControlFlowEquivalent(F, AM, prevLoop, *lit) 
                and hasSameTripCount(F, AM, prevLoop, *lit)
                and not hasNegativeDistanceDependencies(F, AM, prevLoop, *lit);
        
        errs() << optimizable << "\n";
        
        prevLoop = optimizable ? optimize(F, AM, prevLoop, *lit) : *lit;

    }

    return PreservedAnalyses::all();
}
