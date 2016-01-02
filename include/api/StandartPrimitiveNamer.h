//
//  StandartPrimitiveNamer.h
//  SFSL
//
//  Created by Romain Beguet on 02.01.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_StandartPrimitiveNamer__
#define __SFSL__API_StandartPrimitiveNamer__

#include <iostream>
#include "PrimitiveNamer.h"

namespace sfsl {

class SFSL_API_PUBLIC StandartPrimitiveNamer : public common::PrimitiveNamer {
public:

    StandartPrimitiveNamer(const std::string& commonPath, char delimiter,
                             const std::string& unitTypeName, const std::string& boolTypeName, const std::string& intTypeName,
                             const std::string& realTypeName, const std::string& stringTypeName, const std::string& funcTypeName);

    virtual ~StandartPrimitiveNamer();

    virtual Path Unit() const override;
    virtual Path Bool() const override;
    virtual Path Int() const override;
    virtual Path Real() const override;
    virtual Path String() const override;
    virtual Path Func(size_t nbArgs) const override;

    static StandartPrimitiveNamer* const DefaultPrimitiveNamer;

private:

    Path _unitTypePath;
    Path _boolTypePath;
    Path _intTypePath;
    Path _realTypePath;
    Path _stringTypePath;
    Path _funcTypePath;

};

}

#endif
