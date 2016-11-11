#include <iostream>
#include <functional>
#include "AbstractReporter.h"
#include "AbstractPrimitiveNamer.h"

#define SFSL_OPTION_BODY(name, ...) \
    typedef std::tuple<__VA_ARGS__> Params; \
    static std::string getName() { return #name; }

#define SFSL_DEF_OPTION(name, ...) \
    struct SFSL_API_PUBLIC name { \
        SFSL_OPTION_BODY(name, __VA_ARGS__) \
    }; \


namespace sfsl {

namespace opt {

SFSL_DEF_OPTION(Reporter, AbstractReporter*)
SFSL_DEF_OPTION(PrimitiveNamer, common::AbstractPrimitiveNamer*)
SFSL_DEF_OPTION(InitialChunkSize, size_t)

struct SFSL_API_PUBLIC AfterEachPhase {
    enum PrintOption {
        ExecutionTime   = 1 << 0,
        MemoryInfos     = 1 << 1
    };

    typedef std::function<void(const std::string&, double, const std::string&)> ReportingFunction;
    SFSL_OPTION_BODY(AfterEachPhase, ReportingFunction)

    static ReportingFunction print(std::ostream& stream, int printOptions);
};

struct SFSL_API_PUBLIC AtEnd {
    enum PrintOption {
        CompilationTime = 1 << 0,
        MemoryInfos     = 1 << 1
    };

    typedef std::function<void(double, const std::string&)> ReportingFunction;
    SFSL_OPTION_BODY(AtEnd, ReportingFunction)

    static ReportingFunction print(std::ostream& stream, int printOptions);
};

}

}
