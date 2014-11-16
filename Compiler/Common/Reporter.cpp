//
//  Reporter.cpp
//  SFSL
//
//  Created by Romain Beguet on 15.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Reporter.h"

namespace sfsl {

namespace common {

AbstractReporter::~AbstractReporter() {

}

StandartErrReporter::StandartErrReporter() {

}

StandartErrReporter::~StandartErrReporter() {

}

void StandartErrReporter::info(const Positionnable &pos, const std::string &msg) {
    reportMessage("info", pos, msg);
}

void StandartErrReporter::warning(const Positionnable &pos, const std::string &msg) {
    reportMessage("warning", pos, msg);
}

void StandartErrReporter::error(const Positionnable &pos, const std::string &msg) {
    reportMessage("error", pos, msg);
}

void StandartErrReporter::fatal(const Positionnable &pos, const std::string &msg) {
    reportMessage("fatal", pos, msg);
    throw CompilationFatalError(msg);
}

void StandartErrReporter::reportMessage(const std::string &prefix, const Positionnable& pos, const std::string &msg) {
    std::cerr << *pos.getSourceName() << ":" << pos.getPosition()  << ": " << prefix << ": " << msg << std::endl;
}

}

}
