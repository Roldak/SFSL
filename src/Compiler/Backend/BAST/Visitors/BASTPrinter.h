//
//  BASTPrinter.h
//  SFSL
//
//  Created by Romain Beguet on on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__BASTPrinter__
#define __SFSL__BASTPrinter__

#include "BASTVisitor.h"

namespace sfsl {

namespace bast {

class BASTPrinter : public BASTVisitor {
public:

    BASTPrinter(CompCtx_Ptr& ctx, std::ostream& ostream);

    virtual ~BASTPrinter();

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
    virtual void visit(MethodCall* methcall) override;
    virtual void visit(Instantiation* inst) override;

    virtual void visit(BoolLitteral* boollit) override;
    virtual void visit(IntLitteral* intlit) override;
    virtual void visit(RealLitteral* reallit) override;
    virtual void visit(StringLitteral* strlit) override;

private:
    void printIndents();

    size_t _indentCount;

    std::ostream& _ostream;
};

}

}

#endif
