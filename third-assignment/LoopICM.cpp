#include  "llvm/Transforms/Utils/LoopICM.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/Support/GenericLoopInfo.h"
#include <unordered_set>
#include <vector>
#include <functional>

using namespace llvm;

void bulletPoint(const std::string &&name, const BasicBlock *bb, bool printBB = false) {
  errs() << "- " << name << " [first instr; last instr]:";
  bb->front().print(errs());
  errs() << ";";
  bb->back().print(errs());
  if (printBB) bb->print(errs());
  errs() << "\n";
}

void infoLog(Loop &l) {
  errs() << "Loop is " << (l.isLoopSimplifyForm() ? "" : "not ") << "in normal form\n";

  errs() << "Loop blocks: \n";
  if (auto preheader = l.getLoopPreheader()) bulletPoint("preheader", preheader);
  if (auto header = l.getHeader()) bulletPoint("header", header);
  if (auto tail = l.getBlocks().back()) bulletPoint("tail", tail);
  errs() << "Goes through " << std::distance(l.block_begin(), l.block_end()) << " blocks:\n";

  for (auto i = 0u; const auto loopBB : l.blocks()) {
    bulletPoint("BB" + std::to_string(i++), loopBB);
  }
}

bool licmOptimize(Loop &l, LoopStandardAnalysisResults &lar) {
  using namespace std;
  // Enable the following code to get the same
  constexpr bool removeDeadDefinitions = true;

  auto loopInvariantInstructions = unordered_set<Instruction*>{};
  auto liiDiscoveryOrder = vector<Instruction*>{};  ///< Must be order preserving

  // Be aware that this lambda marks phi-instructions as loop-invariant
  auto isOperandLI = [&l, &loopInvariantInstructions] (const Use &usee) {
    if (isa<Constant>(usee)) return true;
    if (isa<Argument>(usee)) return true;
    
    decltype(auto) i = dyn_cast<Instruction>(usee);
    
    if (i and loopInvariantInstructions.contains(i)) return true;
    if (i and not l.contains(i)) return true;

    return false;
  };

  auto isInstructionLI = [&isOperandLI](const Instruction &i) {
    if (isa<PHINode>(i)) return false;

    bool isInstrLI = true;
    // If the loop-invariance condition holds for each operand, the instruction
    // itself is loop-invariant.
    for (const auto &usee : i.operands()) isInstrLI &= isOperandLI(usee);
    return isInstrLI;
  };
  
  // Extract loop-invariant instructions iteratively
  for (auto *bb : l.blocks()) {
    for (auto &i : *bb) {
      if (not isInstructionLI(i)) continue;
      loopInvariantInstructions.insert(&i);
      liiDiscoveryOrder.emplace_back(&i);
    }
  }

  // Printing loop-invariant instructions
  errs() << "LOOP-INVARIANT INSTRUCTIONS\n";
  for (const auto *i : loopInvariantInstructions) {
    i->print(errs());
    errs() << "\n";
  }

  // Extract exit blocks from loop
  using Edge = pair<BasicBlock*, BasicBlock*>;
  auto exitEdges = SmallVector<Edge>{};
  l.getExitEdges(exitEdges);
  const auto &dominators = lar.DT;

  auto isDeadOutsideLoop = [&l] (const Instruction *instr) {
    for (auto u : instr->users()) {
      decltype(auto) i = dyn_cast<Instruction>(u);
      if (not l.contains(i)) return false;
    }

    return true;
  };

  auto isMovable = [&exitEdges, &dominators, &isDeadOutsideLoop] (const Instruction *instr) {
    for (const auto &edge : exitEdges) {
      if (
        not dominators.dominates(instr, edge.second)  // (BBinside, BBoutside)
        and not isDeadOutsideLoop(instr)
      ) return false;
    }
    
    return true;
  };

  // Discarding non-movable instructions
  liiDiscoveryOrder.erase(
    remove_if(begin(liiDiscoveryOrder), end(liiDiscoveryOrder), not_fn(isMovable)),
    end(liiDiscoveryOrder)
  );

  // Printing movable instructions
  errs() << "MOVABLE INSTRUCTIONS\n";
  for (const auto *i : liiDiscoveryOrder) {
    i->print(errs());
    errs() << "\n";
  }

  decltype(auto) preheader = l.getLoopPreheader();

  // Check that instr usees (use-definition chain) have been already moved
  function<bool(Instruction*)> dependenciesSatisfied =
    [&loopInvariantInstructions, &liiDiscoveryOrder, &dependenciesSatisfied] (Instruction *instr) {
      // Base case
      if (not loopInvariantInstructions.contains(instr)) return true;
      if (find(begin(liiDiscoveryOrder), end(liiDiscoveryOrder), instr) == end(liiDiscoveryOrder)) return false;

      // Recursive case
      for (auto &op : instr->operands()) {
        auto *opInstr = dyn_cast<Instruction>(op);
        if (opInstr and not dependenciesSatisfied(opInstr)) return false;
      }

      return true;
  };
  
  errs() << "MOVED\n";
  bool optimized = false;
  for (auto *i : liiDiscoveryOrder) {
    if (not dependenciesSatisfied(i)) continue;
    i->print(errs()); errs() << "\n";


    if constexpr (removeDeadDefinitions) {
      if (i->getNumUses() == 0) {
        i->eraseFromParent();
        continue;
      }
    }

    i->moveBefore(preheader->getTerminator());
    optimized = true;
  }

  return optimized;
}

PreservedAnalyses LoopICM::run(Loop &l, LoopAnalysisManager &lam, LoopStandardAnalysisResults &lar, LPMUpdater &lu) {
  // infoLog(l);

  bool somethingOptimized = licmOptimize(l, lar);
  return somethingOptimized ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
