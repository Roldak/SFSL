//
//  BASTExplicitVisitor.h
//  SFSL
//
//  Created by Romain Beguet on on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__BASTExplicitVisitor__
#define __SFSL__BASTExplicitVisitor__

#include "BASTVisitor.h"

namespace sfsl {

namespace bast {

class BASTExplicitVisitor : public BASTVisitor {
public:

    BASTExplicitVisitor();

    virtual ~BASTExplicitVisitor();

    virtual void visit(BASTNode* node) override;
    virtual void visit(Program* prog) override;

    virtual void visit(Definition* def) override;
    virtual void visit(MethodDef* meth) override;
    virtual void visit(ClassDef* clss) override;
    virtual void visit(GlobalDef* global) override;

    virtual void visit(Expression* expr) override;
    virtual void visit(Block* block) override;
    virtual void visit(DefIdentifier* defid) override;
    virtual void visit(VarIdentifier* varid) override;
    virtual void visit(FieldAccess* fieldacc) override;
    virtual void visit(FieldAssignmentExpression* fassign) override;
    virtual void visit(VarAssignmentExpression* vassign) override;
    virtual void visit(IfExpression* ifexpr) override;
    virtual void visit(DynamicMethodCall* dmethcall) override;
    virtual void visit(StaticMethodCall* smethcall) override;
    virtual void visit(Instantiation* inst) override;

    virtual void visit(UnitLiteral* unitlit) override;
    virtual void visit(BoolLiteral* boollit) override;
    virtual void visit(IntLiteral* intlit) override;
    virtual void visit(RealLiteral* reallit) override;
    virtual void visit(StringLiteral* strlit) override;

};

}

}

#endif
