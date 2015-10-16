//
//  Phase.h
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Phase__
#define __SFSL__API_Phase__

#include <iostream>
#include <vector>
#include <memory>
#include "PhaseContext.h"
#include "SetVisibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC Phase {
public:
    virtual ~Phase();

    virtual std::string runsRightAfter() const;
    virtual std::string runsRightBefore() const;
    virtual std::vector<std::string> runsAfter() const;
    virtual std::vector<std::string> runsBefore() const;

    virtual bool run(PhaseContext& pctx) = 0;

    const std::string& getName() const;
    const std::string& getDescr() const;

    static std::shared_ptr<Phase> PrettyPrint(std::ostream& stream);
    static std::shared_ptr<Phase> StopRightAfter(const std::string& phase);

protected:

    Phase(const std::string& name, const std::string& descr);

private:

    const std::string _name;
    const std::string _descr;
};

}

#endif
