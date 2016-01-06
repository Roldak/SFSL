//
//  StandartPrimitiveNamer.cpp
//  SFSL
//
//  Created by Romain Beguet on 02.01.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "api/StandartPrimitiveNamer.h"
#include "Utils/Utils.h"

namespace sfsl {

StandartPrimitiveNamer* const StandartPrimitiveNamer::DefaultPrimitiveNamer =
        new StandartPrimitiveNamer("sfsl.lang", '.', "unit", "bool", "int", "real", "string", "Func");

StandartPrimitiveNamer::StandartPrimitiveNamer(const std::string& commonPath, char delimiter,
                                                   const std::string& unitTypeName, const std::string& boolTypeName,
                                                   const std::string& intTypeName, const std::string& realTypeName,
                                                   const std::string& stringTypeName, const std::string& funcTypeName) {

    utils::split(_unitTypePath,     commonPath + delimiter + unitTypeName, delimiter);
    utils::split(_boolTypePath,     commonPath + delimiter + boolTypeName, delimiter);
    utils::split(_intTypePath,      commonPath + delimiter + intTypeName, delimiter);
    utils::split(_realTypePath,     commonPath + delimiter + realTypeName, delimiter);
    utils::split(_stringTypePath,   commonPath + delimiter + stringTypeName, delimiter);
    utils::split(_funcTypePath,     commonPath + delimiter + funcTypeName, delimiter);
}

StandartPrimitiveNamer::~StandartPrimitiveNamer() {

}

common::AbstractPrimitiveNamer::Path StandartPrimitiveNamer::Unit() const {
    return _unitTypePath;
}

common::AbstractPrimitiveNamer::Path StandartPrimitiveNamer::Bool() const {
    return _boolTypePath;
}

common::AbstractPrimitiveNamer::Path StandartPrimitiveNamer::Int() const {
    return _intTypePath;
}

common::AbstractPrimitiveNamer::Path StandartPrimitiveNamer::Real() const {
    return _realTypePath;
}

common::AbstractPrimitiveNamer::Path StandartPrimitiveNamer::String() const {
    return _stringTypePath;
}

common::AbstractPrimitiveNamer::Path StandartPrimitiveNamer::Func(size_t nbArgs) const {
    Path tmp = _funcTypePath;
    tmp.back() += utils::T_toString(nbArgs);
    return tmp;
}

}
