#include "sfsl.h"
#include <stdexcept>

int main() {
    sfsl::CompilerConfig config(2048);
    sfsl::Compiler cmp(config);
    sfsl::ProgramBuilder builder = cmp.parse("test",
                                         "module sfsl {"
                                             "module lang {"
                                                 "type unit = class {}"
                                                 "type bool = class {}"
                                                 "type byte = class {}"
                                                 "type int  = class {}"
                                                 "type real = class {}"
                                                 "type string = class {}"
                                             "}"
                                         "}"
                                         "module program { def main() => {x: int; f(x);} }");

    sfsl::Module mod = builder.openModule("program");
    mod.typeDef("int", cmp.parseType("sfsl.lang.int"));
    mod.def("f", cmp.parseType("(int)->sfsl.lang.real"));

    try {
        for (const std::string& str : cmp.compile(builder)) {
            std::cout << str << std::endl;
        }
    } catch (const sfsl::CompileError& err) {
        std::cout << err.what() << std::endl;
    }

    return 0;
}
