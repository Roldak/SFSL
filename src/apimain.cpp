#include "sfsl.h"
#include <stdexcept>

int main() {
    sfsl::CompilerConfig config(2048);
    sfsl::Compiler cmp(config);
    sfsl::ProgramBuilder builder = cmp.parse("test",
                                         "module program { using sfsl.lang def main() => {"
                                             "v: vec2f; v.x = 2.5; v.y = math.pi;"
                                             "math.sin(v.y);"
                                         "} }");

    sfsl::Module slang = builder.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", cmp.classBuilder("unit").build());
    slang.typeDef("bool", cmp.classBuilder("bool").build());
    slang.typeDef("int", cmp.classBuilder("int").build());
    slang.typeDef("real", cmp.classBuilder("real").build());
    slang.typeDef("string", cmp.classBuilder("string").build());

    sfsl::Module mod = builder.openModule("math");
    sfsl::Type real = cmp.parseType("sfsl.lang.real");

    slang.typeDef("vec2f", cmp.classBuilder("vec2f").addField("x", real).addField("y", real).build());

    mod.externDef("pi", real);
    mod.externDef("sin", cmp.createFunctionType({real}, real));

    try {
        for (const std::string& str : cmp.compile(builder)) {
            std::cout << str << std::endl;
        }
    } catch (const sfsl::CompileError& err) {
        std::cout << err.what() << std::endl;
    }

    return 0;
}
