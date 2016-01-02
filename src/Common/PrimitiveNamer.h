//
//  PrimitiveNamer.h
//  SFSL
//
//  Created by Romain Beguet on 02.01.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__PrimitiveNamer__
#define __SFSL__PrimitiveNamer__

#include <iostream>
#include <vector>

namespace sfsl {

namespace common {

class PrimitiveNamer {
public:

    typedef std::vector<std::string> Path;

    virtual ~PrimitiveNamer();

    virtual Path Unit() const = 0;
    virtual Path Bool() const = 0;
    virtual Path Int() const = 0;
    virtual Path Real() const = 0;
    virtual Path String() const = 0;
    virtual Path Func(size_t nbArgs) const = 0;

private:
};

class CommonPathPrimitiveNamer : public PrimitiveNamer {
public:

    CommonPathPrimitiveNamer(const std::string& commonPath, char delimiter,
                             const std::string& unitTypeName, const std::string& boolTypeName, const std::string& intTypeName,
                             const std::string& realTypeName, const std::string& stringTypeName, const std::string& funcTypeName);

    virtual ~CommonPathPrimitiveNamer();

    virtual Path Unit() const override;
    virtual Path Bool() const override;
    virtual Path Int() const override;
    virtual Path Real() const override;
    virtual Path String() const override;
    virtual Path Func(size_t nbArgs) const override;

private:

    Path _unitTypePath;
    Path _boolTypePath;
    Path _intTypePath;
    Path _realTypePath;
    Path _stringTypePath;
    Path _funcTypePath;

};

}

}

#endif
