/***************************************************************************

  jit_gambas_pass.cpp

  gb.jit component

  (c) 2012 Emil Lenngren <emil.lenngren [at] gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include "llvm/Pass.h"
#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 3)
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#elif (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 2)
#include "llvm/IRBuilder.h"
#include "llvm/Function.h"
#else
#include "llvm/Support/IRBuilder.h"
#include "llvm/Function.h"
#endif
#include "llvm/Support/raw_ostream.h"

#include "main.h"

#ifdef __CYGWIN__
#define __finite finite
#define __isnan __isnand
#define __isinf __isinfd
#endif
#define FUNCTION_NAME(s) _FN(s)
#define _FN(s) #s

using namespace llvm;

namespace {

struct GambasPass : public FunctionPass {
	static char ID;
	GambasPass() : FunctionPass(ID) {}
	
	virtual bool runOnFunction(Function &F);
};

}

char GambasPass::ID = 0;

static RegisterPass<GambasPass> X("gamabs-pass", "Gambas Pass", false, false);

FunctionPass* createGambasPass(){
	return new GambasPass();
}

bool GambasPass::runOnFunction(Function &F){
	IRBuilder<> Builder(F.getContext());
	
	bool changed = false;
	for(Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
		for(BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ){
			ICmpInst* ICI = dyn_cast<ICmpInst>(I);
			CallInst* CI = dyn_cast<CallInst>(I++);
			
			if (ICI && ICI->hasMetadata() && ICI->getMetadata("unref_slt") && dyn_cast<LoadInst>(ICI->getOperand(0))){
				ICI->replaceAllUsesWith(ConstantInt::get(ICI->getType(), false));
				ICI->eraseFromParent();
				changed = true;
				continue;
			}
			
			if (!CI)
				continue;
			
			Function* callee = CI->getCalledFunction();
			if (callee == NULL || !callee->isDeclaration())
				continue;
			
			StringRef name = callee->getName();
			if (name == "JR_release_variant" || name == "JR_borrow_variant"){
				ConstantInt* vtype_int = dyn_cast<ConstantInt>(CI->getArgOperand(0));
				if (!vtype_int)
					continue;
				
				uint64_t vtype = vtype_int->getZExtValue();
				if (TYPE_is_string(vtype) || TYPE_is_object(vtype))
					continue;
				
				CI->eraseFromParent();
				changed = true;
			} else if (name == FUNCTION_NAME(__finite)){
				ConstantFP* op = dyn_cast<ConstantFP>(CI->getArgOperand(0));
				if (!op)
					continue;
				
				int val = __finite(op->getValueAPF().convertToDouble());
				Constant* res = ConstantInt::get(CI->getType(), val);
				CI->replaceAllUsesWith(res);
				CI->eraseFromParent();
				changed = true;
			} else if (name == FUNCTION_NAME(__isnan)){
				ConstantFP* op = dyn_cast<ConstantFP>(CI->getArgOperand(0));
				if (!op)
					continue;
				
				int val = __isnan(op->getValueAPF().convertToDouble());
				Constant* res = ConstantInt::get(CI->getType(), val);
				CI->replaceAllUsesWith(res);
				CI->eraseFromParent();
				changed = true;
			} else if (name == FUNCTION_NAME(__isinf)){
				ConstantFP* op = dyn_cast<ConstantFP>(CI->getArgOperand(0));
				if (!op)
					continue;
				
				int val = __isinf(op->getValueAPF().convertToDouble());
				Constant* res = ConstantInt::get(CI->getType(), val);
				CI->replaceAllUsesWith(res);
				CI->eraseFromParent();
				changed = true;
			}
		}
	}
	return changed;
}
