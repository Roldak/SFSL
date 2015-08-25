//
//  TakeSecondIterator.h
//  SFSL
//
//  Created by Romain Beguet on 25.08.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TakeSecondIterator__
#define __SFSL__TakeSecondIterator__

#include <iterator>

namespace sfsl {

namespace utils {

template<typename Iterator>
/**
 * @brief An iterator adapter which returns
 * the second member of the underlying iterator's value
 */
class TakeSecondIterator : public std::iterator<std::input_iterator_tag, typename Iterator::value_type> {
public:
    TakeSecondIterator(const Iterator& it)
        : _it(it) { }

    const typename Iterator::value_type::second_type& operator *() const {
        return _it->second;
    }

    TakeSecondIterator<Iterator>& operator ++() {
        ++_it;
        return *this;
    }

    bool operator ==(const TakeSecondIterator<Iterator>& other) const {
        return _it == other._it;
    }

    bool operator !=(const TakeSecondIterator<Iterator>& other) const {
        return _it != other._it;
    }

private:

    Iterator _it;
};

}

}

#endif
