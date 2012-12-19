// I found no easy way to compile jit_gambas_pass.cpp with -no-rtti as CXXFLAGS while still
// have rtti enabled for jit_codegen.cpp ...

extern "C" {
	void* _ZTIN4llvm12FunctionPassE;
}
