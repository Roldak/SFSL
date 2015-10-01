//
//  ReporterAdapter.cpp
//  SFSL
//
//  Created by Romain Beguet on 01.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "ReporterAdapter.h"
#include "Compiler/Common/Positionnable.h"

namespace sfsl {

ReporterAdapter::ReporterAdapter(sfsl::AbstractReporter* rep) : _apirep(rep) {
    _errorCount = 0;
}

ReporterAdapter::~ReporterAdapter() {

}

void ReporterAdapter::info(const common::Positionnable& pos, const std::string& msg) {
    _apirep->info(pos.getSourceName().getName(), pos.getStartPosition(), pos.getEndPosition(), msg);
}

void ReporterAdapter::warning(const common::Positionnable& pos, const std::string& msg) {
    _apirep->warning(pos.getSourceName().getName(), pos.getStartPosition(), pos.getEndPosition(), msg);
}

void ReporterAdapter::error(const common::Positionnable& pos, const std::string& msg) {
    ++_errorCount;
    _apirep->error(pos.getSourceName().getName(), pos.getStartPosition(), pos.getEndPosition(), msg);
}

void ReporterAdapter::fatal(const common::Positionnable& pos, const std::string& msg) {
    ++_errorCount;
    _apirep->fatal(pos.getSourceName().getName(), pos.getStartPosition(), pos.getEndPosition(), msg);
    AbstractReporter::fatal(pos, msg);
}

}
