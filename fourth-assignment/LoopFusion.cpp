#include "llvm/Transforms/Utils/LoopFusion.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

BasicBlock* getPreheaderOrGuard(Loop *l) {
    return l->isGuarded() ?
        l->getLoopGuardBranch()->getParent()
        : l->getLoopPreheader();
}

bool isAdjacent(Loop *prevLoop, Loop *nextLoop){
    // if the loop has multiple exits the function returns null 
    auto prevExitBB = prevLoop->isGuarded() ? 
        prevLoop->getExitBlock()->getSingleSuccessor()  // Get block after critical edge
        : prevLoop->getExitBlock();

    // loop with multiple exits can't be fused
    if (not prevExitBB) return false; 

    // if the loop is guarded the entry block to check 
    // is the block which contains the guard branch
    auto nextEntryBB = getPreheaderOrGuard(nextLoop);
    
    // prevExitBB->print(errs());
    // nextPreheader->print(errs());

    auto ret = (prevExitBB == nextEntryBB);
    errs() << "Are adjacent? " << ret << "\n";
    return ret;
}

bool isControlFlowEquivalent(Function &F, FunctionAnalysisManager &AM, Loop *prevLoop, Loop *nextLoop){
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    
    auto prevPreheader = getPreheaderOrGuard(prevLoop);
    auto nextPreheader = getPreheaderOrGuard(nextLoop);

    // return true iff the prevLoop dominates the nextLoop
    // and iff the nextLoop postdominates the prevLoop
    auto ret = DT.dominates(prevPreheader, nextPreheader) and PDT.dominates(nextPreheader, prevPreheader);
    errs() << "Are CF equivalent? " << ret << "\n";
    return ret;
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
    auto ret = SE.isKnownPredicate(ICmpInst::ICMP_EQ, prevBackedgeCount, nextBackedgeCount);
    errs() << "Have same TC? " << ret << "\n";
    return ret;
}

/**
 * Checks for any R-W, W-W, or W-R dependency between the pair of loops passed to the function.
 * As in the original LLVM's loop-fusion pass, there is no real distinction between negative/positive/neutral dependencies.
 * Any form of dependcy is prohibited in order to activate the optimization. 
*/
bool hasNotNegativeDistanceDependencies(Function &F, FunctionAnalysisManager &AM, Loop *prevLoop, Loop *nextLoop){
    errs() << "Checking negative dependencies\n";
    
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);
    SmallVector<Instruction*> prevStores, prevLoads, nextLoads, nextStores;

    auto areDependent = [&DI] (Instruction *x, Instruction *y) {
        return DI.depends(x, y, true);
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
            if (i.mayReadFromMemory()) nextLoads.push_back(&i);
        }
    }

    // Store-load, Load-store access patterns can cause problems
    for (auto &prevI : prevStores) {
        for (auto &nextI : nextStores) if (areDependent(prevI, nextI)) return false;
        for (auto &nextI : nextLoads) if (areDependent(prevI, nextI)) return false;
    }

    for (auto &nextI : nextStores) {
        for (auto &prevI : prevStores) if (areDependent(prevI, nextI)) return false;
        for (auto &prevI : prevLoads) if (areDependent(prevI, nextI)) return false;
    }

    return true;
}

Loop * optimize(Function &F, FunctionAnalysisManager &AM, LoopInfo &LI, Loop *prevLoop, Loop *nextLoop){
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    errs() << "Starting the Loop Fusion\n";

    // Get CFG blocks 
    auto prevPreheder = prevLoop->getLoopPreheader();
    auto prevLatch = prevLoop->getLoopLatch();
    auto prevBody = prevLatch->getSinglePredecessor();
    auto prevBodyEntry = prevPreheder->getSingleSuccessor();
    auto prevGuard = prevLoop->getLoopGuardBranch();

    auto nextPreheader = nextLoop->getLoopPreheader();
    auto nextLatch = nextLoop->getLoopLatch();
    auto nextBody = nextLatch->getSinglePredecessor();
    auto nextBodyEntry = nextPreheader->getSingleSuccessor();
    auto nextExit = nextLoop->getExitBlock();

    // retrieve the BB of the nextLoop's
    auto toBeAdded = nextLoop->getBlocksVector();
    
    // remove the nextLoop's latch to keep only the body BB
    toBeAdded.erase(remove(toBeAdded.begin(), toBeAdded.end(), nextLatch), toBeAdded.end());
    toBeAdded.push_back(nextPreheader);

    // Update next loop's induction var with the one from prev loop
    auto prevIV = prevLoop->getInductionVariable(SE);
    auto prevValueIV = dyn_cast<Value>(prevIV);

    auto nextIV = nextLoop->getInductionVariable(SE);
    auto nextValueIV = dyn_cast<Value>(nextIV);
    
    nextValueIV->replaceAllUsesWith(prevValueIV);
    nextIV->eraseFromParent();

    // Move phi nodes from next loop's body to prev one
    nextBodyEntry->replacePhiUsesWith(nextLatch, prevLatch);
    nextBodyEntry->replacePhiUsesWith(nextPreheader, prevPreheder);
    nextPreheader->replacePhiUsesWith(nextPreheader->getSinglePredecessor(), prevBody);
    nextExit->replacePhiUsesWith(nextLatch, prevLatch);
    
    SmallVector<Instruction*> toBeMoved;
    for (Instruction &nextInst : *nextBodyEntry) {
        nextInst.print(errs());
        errs() << "\n";
        if (isa<PHINode>(nextInst)) toBeMoved.emplace_back(&nextInst);
    }

    const auto &movePoint = prevBodyEntry->getFirstNonPHI();
    for (auto *i : toBeMoved) i->moveBefore(movePoint);

    // Edit CFG to reflect fusion changes
    prevLatch->getTerminator()->setSuccessor(1, nextExit);
    prevBody->getTerminator()->replaceSuccessorWith(prevLatch, nextPreheader);
    nextBody->getTerminator()->replaceSuccessorWith(nextLatch, prevLatch);
    nextLatch->getTerminator()->replaceSuccessorWith(nextExit, nextLatch);
    if(prevGuard) prevGuard->setSuccessor(1, nextExit);

    // Clean up unreachable blocks
    EliminateUnreachableBlocks(F);

    // erase nextLoop and add its body to prevLoop
    LI.erase(nextLoop);
    for (auto *bb : toBeAdded)
        prevLoop->addBasicBlockToLoop(bb, LI);

    // Return the first loop of the fused block
    return prevLoop;
}


PreservedAnalyses LoopFusion::run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    
    errs() << "Checking conditions\n";

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
                and hasNotNegativeDistanceDependencies(F, AM, prevLoop, *lit);
            
        if(optimizable) modified = true;
        prevLoop = optimizable ? optimize(F, AM, LI, prevLoop, *lit) : *lit;

    }

    return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}