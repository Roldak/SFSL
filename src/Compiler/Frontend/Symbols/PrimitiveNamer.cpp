//
//  PrimitiveNamer.h
//  SFSL
//
//  Created by Romain Beguet on 02.01.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "PrimitiveNamer.h"
#include "Utils/Utils.h"

namespace sfsl {

namespace sym {

PrimitiveNamer::~PrimitiveNamer() {

}

CommonPathPrimitiveNamer::CommonPathPrimitiveNamer(const std::string& commonPath, char delimiter,
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

CommonPathPrimitiveNamer::~CommonPathPrimitiveNamer() {

}

PrimitiveNamer::Path CommonPathPrimitiveNamer::Unit() const {
    return _unitTypePath;
}

PrimitiveNamer::Path CommonPathPrimitiveNamer::Bool() const {
    return _boolTypePath;
}

PrimitiveNamer::Path CommonPathPrimitiveNamer::Int() const {
    return _intTypePath;
}

PrimitiveNamer::Path CommonPathPrimitiveNamer::Real() const {
    return _realTypePath;
}

PrimitiveNamer::Path CommonPathPrimitiveNamer::String() const {
    return _stringTypePath;
}

PrimitiveNamer::Path CommonPathPrimitiveNamer::Func(size_t nbArgs) const {
    Path tmp = _funcTypePath;
    tmp.back() += utils::T_toString(nbArgs);
    return tmp;
}

}

}
