//
//  Reporter.cpp
//  SFSL
//
//  Created by Romain Beguet on 01.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "api/StandartReporter.h"

namespace sfsl {

AbstractReporter::~AbstractReporter() {

}

StandartReporter::StandartReporter(std::ostream& stream) : _ostream(stream) {

}

StandartReporter::~StandartReporter() {

}

void StandartReporter::info(const std::string& sourceName, size_t start, size_t end, const std::string& message) {
    _ostream << sourceName << ":" << start << ":" << end << ":info:" << message;
}

void StandartReporter::warn(const std::string& sourceName, size_t start, size_t end, const std::string& message) {
    _ostream << sourceName << ":" << start << ":" << end << ":warn:" << message;
}

void StandartReporter::error(const std::string& sourceName, size_t start, size_t end, const std::string& message) {
    _ostream << sourceName << ":" << start << ":" << end << ":error:" << message;
}

class EmptyReporterImpl : public AbstractReporter {
public:
    virtual ~EmptyReporterImpl() {

    }

    virtual void info(const std::string&, size_t, size_t, const std::string&) override { }
    virtual void warn(const std::string&, size_t, size_t, const std::string&) override { }
    virtual void error(const std::string&, size_t, size_t, const std::string&) override { }
};

StandartReporter* const StandartReporter::CoutReporter = new StandartReporter(std::cout);
StandartReporter* const StandartReporter::CerrReporter = new StandartReporter(std::cerr);
AbstractReporter* const StandartReporter::EmptyReporter = new EmptyReporterImpl;

}
