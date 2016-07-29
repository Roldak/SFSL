//
//  UsageTrackable.h
//  SFSL
//
//  Created by Romain Beguet on 28.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__UsageTrackable__
#define __SFSL__UsageTrackable__

#include <iostream>
#include <type_traits>

#include "../../../Common/Positionnable.h"

namespace sfsl {

namespace ast {

enum class UsageProperty : char {
    DECLARED    = 1 << 0,
    INITIALIZED = 1 << 1,
    USABLE      = 1 << 2,
    USED        = 1 << 3,
    MUTABLE     = 1 << 4
};

// thanks http://programmers.stackexchange.com/questions/194412/using-scoped-enums-for-bit-flags-in-c

inline UsageProperty operator|(UsageProperty lhs, UsageProperty rhs) {
    typedef std::underlying_type<UsageProperty>::type T;
    return static_cast<UsageProperty>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline UsageProperty& operator|=(UsageProperty& lhs, UsageProperty rhs) {
    lhs = lhs | rhs;
    return lhs;
}

inline UsageProperty operator&(UsageProperty lhs, UsageProperty rhs) {
    typedef std::underlying_type<UsageProperty>::type T;
    return static_cast<UsageProperty>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline UsageProperty& operator&=(UsageProperty& lhs, UsageProperty rhs) {
    lhs = lhs & rhs;
    return lhs;
}

inline UsageProperty operator~(UsageProperty prop) {
    typedef std::underlying_type<UsageProperty>::type T;
    return static_cast<UsageProperty>(~static_cast<T>(prop));
}

/**
 * @brief An interface to keep track of
 * the used state of an object
 */
class UsageTrackable {
protected:

    UsageTrackable();

public:
    void setProperty(UsageProperty property);
    void unsetProperty(UsageProperty property);
    bool hasProperty(UsageProperty property) const;

private:

    UsageProperty _flags;
};

}

}

#endif
