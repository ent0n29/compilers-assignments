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

void licmOptimize(Loop &l, LoopStandardAnalysisResults &lar) {
  using namespace std;

  using Granularity = Instruction*;
  auto loopInvariantInstructions = unordered_set<Granularity>{};

  // Be aware that this lambda marks phi-instructions as loop-invariant
  auto isOperandLI = [&l, &loopInvariantInstructions] (const Use &usee) {
    if (isa<Constant>(usee)) return true;
    if (isa<Argument>(usee)) return true;
    
    decltype(auto) i = dyn_cast<Instruction>(usee);
    
    if (i and loopInvariantInstructions.count(i)) return true;
    if (i and not l.contains(i)) return true;

    return false;
  };

  auto isInstructionLI = [&isOperandLI](const Instruction &i) {
    if (isa<PHINode>(i)) return false;

    bool isInstrLI = true;
    // If the loop-invariance condition holds for each operand
    for (const auto &usee : i.operands()) isInstrLI &= isOperandLI(usee);
    return isInstrLI;
  };
  
  // Extract loop-invariant instructions with iterative algo
  for (auto *bb : l.blocks()) {
    for (auto &i : *bb) {
      if (isInstructionLI(i)) loopInvariantInstructions.insert(&i);
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
    for (const auto &edge : exitEdges)
      if (
        not dominators.dominates(instr, edge.second)
        and not isDeadOutsideLoop(instr)
      ) return false;
    
    return true;
  };

  // Discarding non-movable instructions
  auto movableInstructions =
    vector<Granularity>(
      begin(loopInvariantInstructions),
      end(loopInvariantInstructions)
    );
  movableInstructions.erase(
    remove_if(begin(movableInstructions), end(movableInstructions), not_fn(isMovable)),
    end(movableInstructions)
  );
  unordered_set<Granularity>().swap(loopInvariantInstructions);  // Make loopInvariantInstructions go out of scope

  // Printing movable instructions
  errs() << "MOVABLE INSTRUCTIONS\n";
  for (const auto *i : movableInstructions) {
    i->print(errs());
    errs() << "\n";
  }
}

PreservedAnalyses LoopICM::run(Loop &l, LoopAnalysisManager &lam, LoopStandardAnalysisResults &lar, LPMUpdater &lu) {
  // infoLog(l);

  licmOptimize(l, lar);

  return PreservedAnalyses::all();
}
