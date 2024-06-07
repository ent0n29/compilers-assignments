#include "llvm/Transforms/Utils/LoopFusion.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
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
                            
                            /*errs() << "\nnextLoop: ";
                            nextInst.print(errs());
                            errs() << "\nprevLoop: ";
                            prevInst.print(errs());
                            if(dep) errs() << "\nDirection: " << dep->isDirectionNegative() << "\n";
                            else errs() << "\nnullptr\n";
                            */
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

    errs() << "Starting the Loop Fusion\n"; 

    // replace the induction variable of the second loop with the one of the first
    
    auto prevIV = prevLoop->getCanonicalInductionVariable();
    auto prevValueIV = dyn_cast<Value>(prevIV);

    auto nextIV = nextLoop->getCanonicalInductionVariable();
    auto nextValueIV = dyn_cast<Value>(nextIV);
    
    nextValueIV->replaceAllUsesWith(prevValueIV);


    // change the CFG 

    auto prevHeader = prevLoop->getHeader();
    auto prevLatch = prevLoop->getLoopLatch();
    auto prevBody = prevLatch->getSinglePredecessor();

    auto nextHeader = nextLoop->getHeader();
    auto nextLatch = nextLoop->getLoopLatch();
    auto nextBody = nextLatch->getSinglePredecessor();

    // get the nextLoop's exit block
    auto nextExit = nextLoop->getExitBlock();

    // get the nextLoop's body entry block
    BasicBlock* nextBodyEntry;
    if (nextLoop->contains(nextHeader->getTerminator()->getSuccessor(0)))
        nextBodyEntry = nextHeader->getTerminator()->getSuccessor(0);
    else
        nextBodyEntry = nextHeader->getTerminator()->getSuccessor(1);

    
    // change the successor of the prevHeader
    prevHeader->getTerminator()->replaceSuccessorWith(nextLoop->getLoopPreheader(), nextExit);

    // change the successor of the prevBody
    prevBody->getTerminator()->replaceSuccessorWith(prevLatch, nextBodyEntry);

    // change the successor of the nextHeader
    ReplaceInstWithInst(nextHeader->getTerminator(), BranchInst::Create(nextLatch));

    // change the successor of the nextBody
    nextBody->getTerminator()->replaceSuccessorWith(nextLatch, prevLatch);

    // when optmized the function must return the new fused loop 
    return prevLoop;
}


PreservedAnalyses LoopFusion::run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    
    errs() << "Started Loop Fusion\n";

    Loop *prevLoop = nullptr;
    bool optimizable = false;
    bool modified = false;
    int num = 0;

    for (auto lit = LI.rbegin(); lit != LI.rend(); ++lit){
        errs() << "Analysing loop "<< num++ << "\n";

        if(prevLoop)
            optimizable = isAdjacent(prevLoop, *lit)   
                and isControlFlowEquivalent(F, AM, prevLoop, *lit) 
                and hasSameTripCount(F, AM, prevLoop, *lit)
                and not hasNegativeDistanceDependencies(F, AM, prevLoop, *lit);
            
        if(optimizable) modified = true;
        prevLoop = optimizable ? optimize(F, AM, prevLoop, *lit) : *lit;

    }

    return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}