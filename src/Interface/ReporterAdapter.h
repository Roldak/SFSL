//
//  ReporterAdapter.h
//  SFSL
//
//  Created by Romain Beguet on 01.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_ReporterAdapter__
#define __SFSL__API_ReporterAdapter__

#include "api/AbstractReporter.h"
#include "Compiler/Common/Reporter.h"

namespace sfsl {

class ReporterAdapter : public common::AbstractReporter {
public:

    ReporterAdapter(sfsl::AbstractReporter* rep);
    virtual ~ReporterAdapter();

    virtual void info(const common::Positionnable& pos, const std::string& msg) override;

    virtual void warning(const common::Positionnable& pos, const std::string& msg) override;

    virtual void error(const common::Positionnable& pos, const std::string& msg) override;

    virtual void fatal(const common::Positionnable& pos, const std::string& msg) override;

private:

    sfsl::AbstractReporter* _apirep;
};

}

#endif
