#include "sfsl.h"

using namespace sfsl;

int main() {
	CompilerConfig config;
	config.reporter = new MyReporter(...);
	
	Compiler cmp(config);
	ProgramDefinition progdef = cmp.parse(src);
	Module lang = progdef.openModule("sfsl").openModule("lang");
	
	ClassBuilder classBuilder = cmp.buildClass("vec2")
		.addField("x", "T")
		.addField("x", "T");
	
	MethodSymbol vec2Constr = classBuilder.addConstructor({"T", "T"});
	MethodSymbol vec2Print = classBuilder.addMethod("print", {}, "vec2[T]");
		
	TypeConstructorBuilder tcBuilder = cmp.buildTypeConstructor("vec2")
		.addParameter("T", "*")
		.setBody(classBuilder.build());
	
	lang.defineType("vec2", tcBuilder.build());
	
	vm::Program prog = cmp.compile(progdef);
	vm::VirtualMachine vm(prog);
	
	vm::SymbolLocation printLoc = vm.find(vec2Print);
	
	vm.link(vec2Constr, [=](sfvm::RuntimeCtx& ctx, vm::Value self, vm::Value* args) {
		// set fields of `self` (this) ...
		ctx.callMethod(printLoc, self, args);
	});
	
	vm.link(vec2Print, [](sfvm::RuntimeCtx& ctx, vm::Value self, vm::Value* args) {
		// std::cout << ...
		return self;
	}
}