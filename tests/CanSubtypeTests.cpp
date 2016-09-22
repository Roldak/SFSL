//
//  CanSubtypeTests.h
//  SFSL
//
//  Created by Romain Beguet on 21.09.16.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include <fstream>
#include <algorithm>
#include <sstream>

#include "sfsl.h"
#include "CanSubtypeTests.h"
#include "AbstractTest.h"
#include "../src/Compiler/Frontend/Types/CanSubtype.h"

namespace sfsl {

namespace test {

using namespace type;

class Clazz final : public CanSubtype<Clazz*> {
public:
    Clazz(const std::string& name) : _name(name) {}

    const std::string& getName() const {
        return _name;
    }

private:
    std::string _name;
};

std::string charToString(char c) {
    std::stringstream ss;
    ss << c;
    return ss.str();
}

class SubTypingTest final : public AbstractTest {
public:
    SubTypingTest(const std::string& name, size_t clazzCount)
        : AbstractTest(name), _clazzes(clazzCount), _success(true) {
        for (size_t i = 0; i < clazzCount; ++i) {
            _clazzes[i] = new Clazz(charToString(nameFrom(i)));
            _clazzes[i]->addSpecialSuperType(_clazzes[i], Environment::Empty);
        }
    }
    ~SubTypingTest() {
        for (Clazz* clazz : _clazzes) {
            delete clazz;
        }
    }

    static SubTypingTest* make(const std::string& name, size_t clazzCount) {
        return new SubTypingTest(name, clazzCount);
    }

    SubTypingTest* subs(char a, char b) {
        clazz(a)->addSuperType(clazz(b), Environment::Empty);
        _log += charToString(a) + " <! " + charToString(b) + "; ";
        return this;
    }

    SubTypingTest* specialSubs(char a, char b) {
        clazz(a)->addSpecialSuperType(clazz(b), Environment::Empty);
        _log += charToString(a) + " <-! " + charToString(b) + "; ";
        return this;
    }

    SubTypingTest* assertIsSubtype(char a, char b, size_t expectedInstances = 0) {
        size_t subtypeInstances = clazz(a)->subTypeInstances(clazz(b)).size();

        _log += charToString(a) + " <? " + charToString(b) + ": ";
        if (expectedInstances == 0) {
            if (subtypeInstances > 0) {
                _log += "true; ";
            } else {
                _log += "false; ";
                _success = false;
            }
        } else {
            if (subtypeInstances == expectedInstances) {
                _log += "true";
            } else {
                _log += "false";
                _success = false;
            }
            _log += " (expected " + std::to_string(expectedInstances)
                    + ", found " + std::to_string(subtypeInstances) + "); ";
        }

        return this;
    }

    SubTypingTest* assertIsNotSubtype(char a, char b) {
        bool subtypes = !clazz(a)->subTypeInstances(clazz(b)).empty();
        _log += charToString(a) + " <? " + charToString(b) + ": ";
        if (subtypes) {
            _log += "true; ";
            _success = false;
        } else {
            _log += "false; ";
        }
        return this;
    }

    bool run(AbstractTestLogger& logger) override {
        logger.result(_name, _success, _log);
        return _success;
    }

private:

    static char nameFrom(size_t index) {
        return 'A' + ((char) index);
    }

    static size_t indexFrom(char name) {
        return (size_t)(name - 'A');
    }

    Clazz* clazz(char name) {
        if (name < 'A' || name >= nameFrom(_clazzes.size())) {
            throw new std::runtime_error(std::string("Fatal error in test. Unexpected class name : '") + std::to_string(name) + "'");
        } else {
            return _clazzes[indexFrom(name)];
        }
    }

    std::vector<Clazz*> _clazzes;
    std::string _log;
    bool _success;
};

TestRunner* buildCanSubtypeTests() {
    TestSuiteBuilder basic("Basic");

    basic.addTest(SubTypingTest::make("Subtype of self", 1)->assertIsSubtype('A', 'A'));
    basic.addTest(SubTypingTest::make("Basic #1", 2)->subs('A', 'B')
                  ->assertIsSubtype('A', 'B')->assertIsNotSubtype('B', 'A'));

    TestSuiteBuilder trans("Transitivity");

    trans.addTest(SubTypingTest::make("Transitivity #1", 3)->subs('A', 'B')->subs('B', 'C')
                  ->assertIsSubtype('A', 'C'));

    trans.addTest(SubTypingTest::make("Transitivity #2", 3)->subs('B', 'C')->subs('A', 'B')
                  ->assertIsSubtype('A', 'C'));

    trans.addTest(SubTypingTest::make("Deep Hierarchy", 9)
                  ->subs('A', 'B')->subs('B', 'C')->subs('C', 'D')->subs('D', 'E')
                  ->subs('E', 'F')->subs('F', 'G')->subs('G', 'H')->subs('H', 'I')
                  ->assertIsSubtype('A', 'I')->assertIsSubtype('D', 'G')
                  ->assertIsNotSubtype('G', 'B')->assertIsNotSubtype('I', 'H'));

    TestSuiteBuilder multi("MultipleInheritance");

    multi.addTest(SubTypingTest::make("Multiple #1", 3)->subs('A', 'B')->subs('A', 'C')
                  ->assertIsSubtype('A', 'B')->assertIsSubtype('A', 'C')
                  ->assertIsNotSubtype('B', 'A')->assertIsNotSubtype('C', 'A'));

    multi.addTest(SubTypingTest::make("Diamond #1", 4)
                  ->subs('A', 'B')->subs('A', 'C')->subs('B', 'D')->subs('C', 'D')
                  ->assertIsSubtype('A', 'B')->assertIsSubtype('A', 'C')->assertIsSubtype('A', 'D', 2));

    multi.addTest(SubTypingTest::make("Diamond #2", 4)
                  ->subs('C', 'D')->subs('A', 'C')->subs('B', 'D')->subs('A', 'B')
                  ->assertIsSubtype('A', 'B')->assertIsSubtype('A', 'C')->assertIsSubtype('A', 'D', 2));

    multi.addTest(SubTypingTest::make("Double Diamond #1", 7)
                  ->subs('A', 'B')->subs('A', 'C')->subs('B', 'D')->subs('C', 'D')
                  ->subs('D', 'E')->subs('D', 'F')->subs('E', 'G')->subs('F', 'G')
                  ->assertIsSubtype('A', 'B')->assertIsSubtype('A', 'C')->assertIsSubtype('A', 'D', 2)
                  ->assertIsSubtype('D', 'E')->assertIsSubtype('D', 'F')->assertIsSubtype('D', 'G', 2)
                  ->assertIsSubtype('A', 'E', 2)->assertIsSubtype('A', 'F', 2)->assertIsSubtype('A', 'G', 4));

    multi.addTest(SubTypingTest::make("Double Diamond #2", 7)
                  ->subs('D', 'E')->subs('D', 'F')->subs('E', 'G')->subs('F', 'G')
                  ->subs('A', 'B')->subs('A', 'C')->subs('B', 'D')->subs('C', 'D')
                  ->assertIsSubtype('A', 'B')->assertIsSubtype('A', 'C')->assertIsSubtype('A', 'D', 2)
                  ->assertIsSubtype('D', 'E')->assertIsSubtype('D', 'F')->assertIsSubtype('D', 'G', 2)
                  ->assertIsSubtype('A', 'E', 2)->assertIsSubtype('A', 'F', 2)->assertIsSubtype('A', 'G', 4));

    multi.addTest(SubTypingTest::make("Double Diamond #3", 7)
                  ->subs('A', 'C')->subs('F', 'G')->subs('B', 'D')->subs('D', 'F')
                  ->subs('C', 'D')->subs('D', 'E')->subs('E', 'G')->subs('A', 'B')
                  ->assertIsSubtype('A', 'B')->assertIsSubtype('A', 'C')->assertIsSubtype('A', 'D', 2)
                  ->assertIsSubtype('D', 'E')->assertIsSubtype('D', 'F')->assertIsSubtype('D', 'G', 2)
                  ->assertIsSubtype('A', 'E', 2)->assertIsSubtype('A', 'F', 2)->assertIsSubtype('A', 'G', 4));

    return new TestRunner("CanSubtypeTests", {basic.build(), trans.build(), multi.build()});
}

}

}
