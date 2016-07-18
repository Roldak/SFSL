//
//  BASTPrinter.cpp
//  SFSL
//
//  Created by Romain Beguet on on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#include "BASTPrinter.h"

namespace sfsl {

namespace bast {

BASTPrinter::BASTPrinter(std::ostream& ostream) : _indentCount(0), _ostream(ostream) {

}

BASTPrinter::~BASTPrinter() {

}

void BASTPrinter::visit(BASTNode* node) {

}

void BASTPrinter::visit(Program* prog) {
    printIndents();
    _ostream << "VISIBLE {" << std::endl;
    ++_indentCount;

    for (Definition* def : prog->getVisibleDefinitions()) {
        def->onVisit(this);
    }

    --_indentCount;
    printIndents();
    _ostream << "}" << std::endl << std::endl << "HIDDEN {" << std::endl;

    ++_indentCount;

    for (Definition* def : prog->getHiddenDefinitions()) {
        def->onVisit(this);
    }

    --_indentCount;
    printIndents();
    _ostream << "}" << std::endl;
}

void BASTPrinter::visit(Definition* def) {

}

void BASTPrinter::visit(MethodDef* meth) {
    printIndents();
    _ostream << "meth " << meth->getName() << "(" << meth->getVarCount() << " vars)";
    if (meth->getMethodBody()) {
        _ostream << " => ";
        meth->getMethodBody()->onVisit(this);
    }
    _ostream << std::endl;
}

void BASTPrinter::visit(ClassDef* clss) {
    printIndents();
    _ostream << "class " << clss->getName() << "(" << clss->getFieldCount() << " fields)";
    if (clss->getParent()) {
        _ostream << " : " << clss->getParent()->getValue();
    }
    _ostream << " {" << std::endl;
    ++_indentCount;

    for (DefIdentifier* meth : clss->getMethods()) {
        printIndents();
        _ostream << "meth " << meth->getValue() << std::endl;
    }

    --_indentCount;
    printIndents();
    _ostream << "}" << std::endl;
}

void BASTPrinter::visit(GlobalDef* global) {
    printIndents();
    _ostream << "global " << global->getName();
    if (global->getBody()) {
        _ostream << " = ";
        global->getBody()->onVisit(this);
    }
    _ostream << std::endl;
}

void BASTPrinter::visit(Expression* expr) {

}

void BASTPrinter::visit(Block* block) {
    _ostream << "{" << std::endl;
    ++_indentCount;

    for (Expression* expr : block->getStatements()) {
        printIndents();
        expr->onVisit(this);
        _ostream << ";" << std::endl;
    }

    --_indentCount;
    printIndents();
    _ostream << "}";
}

void BASTPrinter::visit(DefIdentifier* defid) {
    _ostream << defid->getValue();
}

void BASTPrinter::visit(VarIdentifier* varid) {
    _ostream << "{" << varid->getLocalId() << "}";
}

void BASTPrinter::visit(FieldAccess* fieldacc) {
    _ostream << "(";
    fieldacc->getAccessed()->onVisit(this);
    _ostream << ").{" << fieldacc->getFieldId() << "}";
}

void BASTPrinter::visit(FieldAssignmentExpression* fassign) {
    _ostream << "(";
    fassign->getAccessed()->onVisit(this);
    _ostream << ").{" << fassign->getFieldId() << "} = (";
    fassign->getValue()->onVisit(this);
    _ostream << ")";
}

void BASTPrinter::visit(VarAssignmentExpression* vassign) {
    _ostream << "{" << vassign->getAssignedVarLocalId() << "} = (";
    vassign->getValue()->onVisit(this);
    _ostream << ")";
}

void BASTPrinter::visit(IfExpression* ifexpr) {
    _ostream << "if (";
    ifexpr->getCondition()->onVisit(this);
    _ostream << ") (";
    ifexpr->getThen()->onVisit(this);
    _ostream << ") else (";
    ifexpr->getElse()->onVisit(this);
    _ostream << ")";
}

void BASTPrinter::visit(MethodCall* methcall) {
    _ostream << "(";
    methcall->getCallee()->onVisit(this);
    _ostream << ")" << "[" << methcall->getVirtualId() << "](";
    if (methcall->getArgs().size() > 0) {
        for (size_t i = 0; i < methcall->getArgs().size() - 1; ++i) {
            methcall->getArgs()[i]->onVisit(this);
            _ostream << ", ";
        }
        methcall->getArgs().back()->onVisit(this);
    }
    _ostream << ")";
}

void BASTPrinter::visit(Instantiation* inst) {
    _ostream << "new " << inst->getClassId()->getValue();
}

void BASTPrinter::visit(UnitLiteral* unitlit) {
    _ostream << "()";
}

void BASTPrinter::visit(BoolLiteral* boollit) {
    _ostream << (boollit->getValue() ? "true" : "false");
}

void BASTPrinter::visit(IntLiteral* intlit) {
    _ostream << intlit->getValue();
}

void BASTPrinter::visit(RealLiteral* reallit) {
    _ostream << reallit->getValue();
}

void BASTPrinter::visit(StringLiteral* strlit) {
    _ostream << "\"" << strlit->getValue() << "\"";
}

void BASTPrinter::printIndents() {
    for (size_t i = 0; i < _indentCount; ++i) {
        _ostream << "    ";
    }
}

}

}
