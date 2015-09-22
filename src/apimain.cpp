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
                                         "module program { def main() => {math.sin(math.pi);} }");

    sfsl::Module mod = builder.openModule("math");
    mod.externDef("pi", cmp.parseType("sfsl.lang.real"));
    mod.externDef("sin", cmp.parseType("(sfsl.lang.real)->sfsl.lang.real"));

    try {
        for (const std::string& str : cmp.compile(builder)) {
            std::cout << str << std::endl;
        }
    } catch (const sfsl::CompileError& err) {
        std::cout << err.what() << std::endl;
    }

    return 0;
}
