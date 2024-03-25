#include "llvm/IR/PassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

namespace llvm {
	class LocalOpts : public PassInfoMixin<LocalOpts> {
		public:
			PreservedAnalyses run(Module &m, ModuleAnalysisManager &AM);
	};
} // nam
