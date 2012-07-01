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
#include "llvm/Function.h"
#include "llvm/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "main.h"

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
			CallInst* CI = dyn_cast<CallInst>(I++);
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
			} else if (name == "__finite"){
				ConstantFP* op = dyn_cast<ConstantFP>(CI->getArgOperand(0));
				if (!op)
					continue;
				
				int val = __finite(op->getValueAPF().convertToDouble());
				Constant* res = ConstantInt::get(CI->getType(), val);
				CI->replaceAllUsesWith(res);
				CI->eraseFromParent();
				changed = true;
			} else if (name == "__isnan"){
				ConstantFP* op = dyn_cast<ConstantFP>(CI->getArgOperand(0));
				if (!op)
					continue;
				
				int val = __isnan(op->getValueAPF().convertToDouble());
				Constant* res = ConstantInt::get(CI->getType(), val);
				CI->replaceAllUsesWith(res);
				CI->eraseFromParent();
				changed = true;
			} else if (name == "__isinf"){
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
