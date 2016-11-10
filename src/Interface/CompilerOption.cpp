//
//  CompilerOption.cpp
//  SFSL
//
//  Created by Romain Beguet on 10.11.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "api/CompilerOption.h"

namespace sfsl {

namespace opt {

AfterEachPhase::ReportingFunction AfterEachPhase::print(std::ostream& stream, int printOptions) {
    return AfterEachPhase::ReportingFunction([&stream, printOptions](const std::string& phaseName, double executionTime, const std::string& memoryInfos) {
        stream << "Phase " << phaseName << ":" << std::endl;
        if (printOptions & PrintOption::ExecutionTime) {
            stream << "  Execution time: " << executionTime << " sec." << std::endl;
        }
        if (printOptions & PrintOption::MemoryInfos) {
            stream << "  Memory infos: " << memoryInfos << std::endl;
        }
    });
}

AtEnd::ReportingFunction AtEnd::print(std::ostream& stream, int printOptions) {
    return AtEnd::ReportingFunction([&stream, printOptions](double compilationTime, const std::string& memoryInfos) {
        if (printOptions & PrintOption::CompilationTime) {
            stream << "Total compilation time: " << compilationTime << " sec." << std::endl;
        }
        if (printOptions & PrintOption::MemoryInfos) {
            stream << "Final memory infos: " << memoryInfos << std::endl;
        }
    });
}

}

}
