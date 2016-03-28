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

class Definition {
public:
    Definition(const std::string& name);
    virtual ~Definition();

    const std::string& getName() const;

private:

    const std::string _name;
};

class ClassDef final : public Definition {
public:
    class Method final {
    public:
        Method(const std::string& name, size_t argCount);
        Method(const std::string& name, size_t argCount, BASTNode* body);

        const std::string& getName() const;

        size_t getArgCount() const;

        BASTNode* getMethodBody() const;

    private:

        const std::string _name;
        size_t _argCount;
        BASTNode* _body;
    };

    ClassDef(const std::string& name, size_t fieldCount, std::vector<Method>& methods);
    virtual ~ClassDef();

    size_t getFieldCount() const;

    const std::vector<Method>& getMethods() const;

private:

    size_t _fieldCount;
    std::vector<Method> _methods;
};

class GlobalDef final : public Definition {
public:
    GlobalDef(const std::string& name, BASTNode* body);
    virtual ~GlobalDef();

    BASTNode* getBody() const;

private:

    BASTNode* _body;
};

class Program : public BASTNode {
public:
    Program(const std::vector<ClassDef>& classes,
            const std::vector<GlobalDef>& globals);

    virtual ~Program();

    SFSL_BAST_ON_VISIT_H

    const std::vector<ClassDef>& getClasses() const;

    const std::vector<GlobalDef>& getGlobals() const;

private:

    std::vector<ClassDef> _classes;
    std::vector<GlobalDef> _globals;
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

    const std::string _name;
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
 * @brief Represents a function call.
 */
class MethodCall : public Expression {
public:

    MethodCall(Expression* callee, size_t virtualId, const std::vector<Expression*>& args);
    virtual ~MethodCall();

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
     * @return The arguments by extracting them directly from the tuple
     */
    const std::vector<Expression*>& getArgs() const;

private:

    Expression* _callee;
    size_t _virtualId;
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
 * @brief Represents a boolean litteral
 */
class BoolLitteral : public Expression {
public:

    BoolLitteral(const sfsl_bool_t value);
    virtual ~BoolLitteral();

    SFSL_BAST_ON_VISIT_H

    sfsl_bool_t getValue() const;

private:

    const sfsl_bool_t _value;
};

/**
 * @brief Represents an Integer litteral.
 */
class IntLitteral : public Expression {
public:

    IntLitteral(const sfsl_int_t value);
    virtual ~IntLitteral();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The value of the litteral
     */
    sfsl_int_t getValue() const;

private:

    const sfsl_int_t _value;
};

/**
 * @brief Represents a Real litteral
 */
class RealLitteral : public Expression {
public:

    RealLitteral(const sfsl_real_t value);
    virtual ~RealLitteral();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The value of the litteral
     */
    sfsl_real_t getValue() const;

private:

    const sfsl_real_t _value;
};

/**
 * @brief Represents a String litteral
 */
class StringLitteral : public Expression {
public:

    StringLitteral(const std::string& value);
    virtual ~StringLitteral();

    SFSL_BAST_ON_VISIT_H

    /**
     * @return The value of the litteral
     */
    const std::string& getValue() const;

private:

    const std::string _value;
};

}

}

#endif
