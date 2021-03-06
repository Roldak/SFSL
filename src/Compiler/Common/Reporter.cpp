//
//  Reporter.cpp
//  SFSL
//
//  Created by Romain Beguet on 15.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Reporter.h"
#include "Positionnable.h"

namespace sfsl {

namespace common {

AbstractReporter::~AbstractReporter() {

}

void AbstractReporter::fatal(const Positionnable&, const std::string& msg) {
    throw CompilationFatalError(msg);
}

size_t AbstractReporter::getErrorCount() {
    return _errorCount;
}

StandartErrReporter::StandartErrReporter() {
    _errorCount = 0;
}

StandartErrReporter::~StandartErrReporter() {

}

void StandartErrReporter::info(const Positionnable& pos, const std::string& msg) {
    reportMessage("info", pos, msg);
}

void StandartErrReporter::warning(const Positionnable& pos, const std::string& msg) {
    reportMessage("warning", pos, msg);
}

void StandartErrReporter::error(const Positionnable& pos, const std::string& msg) {
    reportMessage("error", pos, msg);
    ++_errorCount;
}

void StandartErrReporter::fatal(const Positionnable& pos, const std::string& msg) {
    reportMessage("fatal", pos, msg);
    ++_errorCount;
    AbstractReporter::fatal(pos, msg);
}

void StandartErrReporter::reportMessage(const std::string& prefix, const Positionnable& pos, const std::string& msg) {
    std::cerr << pos.positionStr() << ":" << prefix << ":" << msg << std::endl;
}

}

}
