#include "sfsl.h"

int main() {
    sfsl::CompilerConfig config{2048};
    sfsl::Compiler cmp(config);
    sfsl::ProgramBuilder builder = cmp.parse("test", "module program { def main() => {} }");


    std::cout << "lala" << std::endl;
    return 0;
}
