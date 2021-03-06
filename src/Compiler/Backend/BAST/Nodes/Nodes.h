//
//  Nodes.h
//  SFSL
//
//  Created by Romain Beguet on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__BAST_Nodes__
#define __SFSL__BAST_Nodes__

#include <iostream>
#include <vector>
#include "BASTNode.h"
#include "../../../../Utils/Utils.h"

namespace sfsl {

namespace bast {

class DefIdentifier;

class Definition : public BASTNode {
public:
    Definition(const std::string& name);
    virtual ~Definition();

    SFSL_BAST_ON_VISIT_H

    const std::string& getName() const;

private:

    std::string _name;
};

class MethodDef final : public Definition {
public:
    MethodDef(const std::string& name, size_t varCount);
    MethodDef(const std::string& name, size_t varCount, BASTNode* body);
    virtual ~MethodDef();

    SFSL_BAST_ON_VISIT_H

    size_t getVarCount() const;

    BASTNode* getMethodBody() const;

private:

    size_t _varCount;
    BASTNode* _body;
};

class ClassDef final : public Definition {
public:

    ClassDef(const std::string& name, size_t fieldCount, DefIdentifier* parent, const std::vector<DefIdentifier*>& methods);
    virtual ~ClassDef();

    SFSL_BAST_ON_VISIT_H

    size_t getFieldCount() const;
    DefIdentifier* getParent() const;
    const std::vector<DefIdentifier*>& getMethods() const;

private:

    size_t _fieldCount;
    DefIdentifier* _parent;
    std::vector<DefIdentifier*> _methods;
};

class GlobalDef final : public Definition {
public:
    GlobalDef(const std::string& name);
    GlobalDef(const std::string& name, BASTNode* body);
    virtual ~GlobalDef();

    SFSL_BAST_ON_VISIT_H

    BASTNode* getBody() const;

private:

    BASTNode* _body;
};

class Program : public BASTNode {
public:
    Program(const std::vector<Definition*>& visibleDefs,
            const std::vector<Definition*>& hiddenDefs);

    virtual ~Program();

    SFSL_BAST_ON_VISIT_H

    const std::vector<Definition*>& getVisibleDefinitions() const;
    const std::vector<Definition*>& getHiddenDefinitions() const;

private:

    std::vector<Definition*> _visibleDefs;
    std::vector<Definition*> _hiddenDefs;
};

/**
 * @brief A superclass that represents an expression.
 * Cannot be constructed. This class is there just to provide
 * a lower upper bound for all the expressions than BASTNode
 */
class Expression : public BASTNode {
public:

    virtual ~Expression();

    SFSL_BAST_ON_VISIT_H

};

/**
 * @brief Represents a block, aka a list of statements
 */
class Block : public Expression {
public:
    Block(const std::vector<Expression*>& stats);
    virtual ~Block();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The list of statements contained in the block
     */
    const std::vector<Expression*>& getStatements() const;

private:

    std::vector<Expression*> _stats;
};

/**
 * @brief Represents an Identifier that refers to a definition.
 */
class DefIdentifier : public Expression {
public:

    DefIdentifier(const std::string& name);
    virtual ~DefIdentifier();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The name of the identifier
     */
    const std::string& getValue() const;

private:

    std::string _name;
};

/**
 * @brief Represents an Identifier that refers to a variable.
 */
class VarIdentifier : public Expression {
public:

    VarIdentifier(size_t localId);
    virtual ~VarIdentifier();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The id of the identifier
     */
    size_t getLocalId() const;

private:

    const size_t _localId;
};

/**
 * @brief Represents a field access
 */
class FieldAccess : public Expression {
public:

    FieldAccess(Expression* accessed, size_t fieldId);
    virtual ~FieldAccess();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The accessed part (the left side)
     */
    Expression* getAccessed() const;

    /**
     * @return The id of the field to access
     */
    size_t getFieldId() const;

private:

    Expression* _accessed;
    size_t _fieldId;
};

/**
 * @brief Represents an assignement expression, aka e.x = 2.
 */
class FieldAssignmentExpression : public Expression {
public:

    FieldAssignmentExpression(Expression* accessed, size_t fieldId, Expression* value);
    virtual ~FieldAssignmentExpression();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The accessed part of the member assignment (e.g. for e.x = 2; accessed = e, member = "x", value = 2)
     */
    Expression* getAccessed() const;

    /**
     * @return The member part (the right side)
     */
    size_t getFieldId() const;

    /**
     * @return The right hand side of the member assignment
     */
    Expression* getValue() const;

private:

    Expression* _accessed;
    size_t _fieldId;
    Expression* _value;
};

/**
 * @brief Represents an assignement expression, aka lhs = rhs.
 */
class VarAssignmentExpression : public Expression {
public:

    VarAssignmentExpression(size_t localId, Expression* value);
    virtual ~VarAssignmentExpression();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The left hand side of the binary expression
     */
    size_t getAssignedVarLocalId() const;

    /**
     * @return The right hand side of the binary expression
     */
    Expression* getValue() const;

private:

    size_t _localId;
    Expression* _value;
};


/**
 * @brief Represents an If expression, e.g. if (1 + 1 == 2) println("hi") else println("salut")
 */
class IfExpression : public Expression {
public:

    IfExpression(Expression* cond, Expression* then, Expression* els);
    virtual ~IfExpression();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The condition expression
     */
    Expression* getCondition() const;

    /**
     * @return The then-part expression
     */
    Expression* getThen() const;

    /**
     * @return The else-part expression, potentially null
     */
    Expression* getElse() const;

private:

    Expression* _cond;
    Expression* _then;
    Expression* _else;
};

/**
 * @brief Represents a dynamic method call (i.e. x.f()).
 */
class DynamicMethodCall : public Expression {
public:

    DynamicMethodCall(Expression* callee, size_t virtualId, const std::vector<Expression*>& args);
    virtual ~DynamicMethodCall();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The expression to which is applied the parentheses operator
     */
    Expression* getCallee() const;

    /**
     * @return The virtual id of the method to call in the class of the callee expression
     */
    size_t getVirtualId() const;

    /**
     * @return The call arguments (without the `this` argument)
     */
    const std::vector<Expression*>& getArgs() const;

private:

    Expression* _callee;
    size_t _virtualId;
    std::vector<Expression*> _args;
};

/**
 * @brief Represents a static method call (i.e. A.f())
 */
class StaticMethodCall : public Expression {
public:

    StaticMethodCall(DefIdentifier* callee, const std::vector<Expression*>& args);
    virtual ~StaticMethodCall();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The method identifier
     */
    DefIdentifier* getCallee() const;

    /**
     * @return The call arguments (with `this` as the first argument)
     */
    const std::vector<Expression*>& getArgs() const;

private:

    DefIdentifier* _callee;
    std::vector<Expression*> _args;
};

/**
 * @brief Represents an instantiation of a type.
 * This node is never created by the parser.
 */
class Instantiation : public Expression {
public:

    Instantiation(DefIdentifier* defId);
    virtual ~Instantiation();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The TypeExpression corresponding to the instantiated type
     */
    DefIdentifier* getClassId() const;

private:

    DefIdentifier* _defId;
};

/**
 * @brief Represents a unit Literal
 */
class UnitLiteral : public Expression {
public:

    UnitLiteral();
    virtual ~UnitLiteral();

    SFSL_BAST_ON_VISIT_H

private:
};

/**
 * @brief Represents a boolean Literal
 */
class BoolLiteral : public Expression {
public:

    BoolLiteral(const sfsl_bool_t value);
    virtual ~BoolLiteral();

    SFSL_BAST_ON_VISIT_H

    sfsl_bool_t getValue() const;

private:

    const sfsl_bool_t _value;
};

/**
 * @brief Represents an Integer Literal.
 */
class IntLiteral : public Expression {
public:

    IntLiteral(const sfsl_int_t value);
    virtual ~IntLiteral();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The value of the Literal
     */
    sfsl_int_t getValue() const;

private:

    const sfsl_int_t _value;
};

/**
 * @brief Represents a Real Literal
 */
class RealLiteral : public Expression {
public:

    RealLiteral(const sfsl_real_t value);
    virtual ~RealLiteral();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The value of the Literal
     */
    sfsl_real_t getValue() const;

private:

    const sfsl_real_t _value;
};

/**
 * @brief Represents a String Literal
 */
class StringLiteral : public Expression {
public:

    StringLiteral(const std::string& value);
    virtual ~StringLiteral();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The value of the Literal
     */
    const std::string& getValue() const;

private:

    const std::string _value;
};

}

}

#endif
