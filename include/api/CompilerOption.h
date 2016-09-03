#include <iostream>
#include "AbstractReporter.h"
#include "AbstractPrimitiveNamer.h"

#define DEF_OPTION(name, ...) \
    struct name { \
        typedef std::tuple<__VA_ARGS__> Params; \
        static std::string getName() { return #name; } \
    }; \


namespace sfsl {

namespace opt {

enum class Frequency {
    Never,
    AfterEachPhase,
    AfterLastPhase
};

DEF_OPTION(Reporter, AbstractReporter*)
DEF_OPTION(PrimitiveNamer, common::AbstractPrimitiveNamer*)
DEF_OPTION(InitialChunkSize, size_t)
DEF_OPTION(PrintMemoryUsage, Frequency)
DEF_OPTION(PrintCompilationTime, Frequency)

}

}
