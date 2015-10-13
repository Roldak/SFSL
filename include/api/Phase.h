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

namespace sfsl {

class Phase {
public:
    virtual ~Phase();

    virtual const std::string runsRightAfter() const;
    virtual const std::string runsRightBefore() const;
    virtual const std::vector<std::string> runsAfter() const;
    virtual const std::vector<std::string> runsBefore() const;

    virtual void run() = 0;

    const std::string& getName() const;
    const std::string& getDescr() const;

    static const std::string NoPhase;
    static const std::vector<std::string> None;

protected:

    Phase(const std::string& name, const std::string& descr);

private:

    const std::string _name;
    const std::string _descr;
};

}

#endif
