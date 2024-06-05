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

bool hasNegativeDistanceDependencies(Function &F, FunctionAnalysisManager &AM, Loop *prevLoop, Loop *nextLoop){
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);
    
    errs() << "Checking negative dependencies\n";
    bool dependent = false;

    //slide every instruction of nextLoop
    for (auto *nextBB : nextLoop->blocks()){
        for (auto &nextInst : *nextBB){
            
            // check dependencies only between loads and stores instructions
            // as loops can have negative distance dependecies only by using arrays
            if(isa<LoadInst>(nextInst) or isa<StoreInst>(nextInst)){
                
                // slide every instruction of prevLoop
                for (auto *prevBB : prevLoop->blocks()){ 
                    for (auto &prevInst : *prevBB) {
                        
                        // check dependencies only for loads and stores avoiding the function's call
                        if(isa<LoadInst>(prevInst) or isa<StoreInst>(prevInst)){
                            
                            auto dep = DI.depends(&prevInst, &nextInst, true);
                            
                            errs() << "\nnextLoop: ";
                            nextInst.print(errs());
                            errs() << "\nprevLoop: ";
                            prevInst.print(errs());
                            errs() <<"\n" << dep->isDirectionNegative() << "\n";

                            if (dep and dep->isDirectionNegative())
                                dependent = true;

                        }
                    }
                }

            }

        }
    }

    return dependent;
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
        
        prevLoop = optimizable ? optimize(F, AM, prevLoop, *lit) : *lit;

    }

    return PreservedAnalyses::all();
}
