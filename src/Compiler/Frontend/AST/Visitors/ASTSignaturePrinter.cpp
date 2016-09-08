//
//  ASTSignaturePrinter.h
//  SFSL
//
//  Created by Romain Beguet on 08.09.16.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTSignaturePrinter.h"

#include <sstream>

namespace sfsl {

namespace ast {

ASTSignaturePrinter::ASTSignaturePrinter(CompCtx_Ptr& ctx, std::ostream& ostream)
    : ASTExplicitVisitor(ctx), _isValid(true), _ostream(ostream) {

}

ASTSignaturePrinter::~ASTSignaturePrinter() {

}

bool ASTSignaturePrinter::printSignatureOf(ASTNode* node, std::ostream& ostream, CompCtx_Ptr& ctx) {
    ASTSignaturePrinter printer(ctx, ostream);
    node->onVisit(&printer);
    return printer._isValid;
}

std::string ASTSignaturePrinter::getSignatureOf(ASTNode* node, bool& isValid, CompCtx_Ptr& ctx) {
    std::ostringstream oss;
    isValid = printSignatureOf(node, oss, ctx);
    return oss.str();
}

void ASTSignaturePrinter::visit(FunctionTypeDecl* ftdecl) {
    if (ftdecl->getTypeArgs().size() > 0) {
        _ostream << "[";

        for (size_t i = 0; i < ftdecl->getTypeArgs().size() - 1; ++i) {
            doVisit(ftdecl->getTypeArgs()[i]);
            _ostream << ", ";
        }
        doVisit(ftdecl->getTypeArgs().back());

        _ostream << "]";
    }
    _ostream << "(";
    if (ftdecl->getArgTypes().size() > 0) {
        for (size_t i = 0; i < ftdecl->getArgTypes().size() - 1; ++i) {
            doVisit(ftdecl->getArgTypes()[i]);
            _ostream << ", ";
        }
        doVisit(ftdecl->getArgTypes().back());
    }
    _ostream << ")->";
    ftdecl->getRetType()->onVisit(this);

    setValid();
}

void ASTSignaturePrinter::visit(TypeConstructorCall* tcall) {
    doVisit(tcall->getCallee());

    _ostream << "[";

    if (tcall->getArgs().size() > 0) {
        for (size_t i = 0; i < tcall->getArgs().size() - 1; ++i) {
            doVisit(tcall->getArgs()[i]);
            _ostream << ", ";
        }
        doVisit(tcall->getArgs().back());
    }

    _ostream << "]";

    setValid();
}

void ASTSignaturePrinter::visit(TypeMemberAccess* mac) {
    _ostream << mac->getSymbol()->getAbsoluteName();

    setValid();
}

void ASTSignaturePrinter::visit(TypeIdentifier* ident) {
    _ostream << ident->getSymbol()->getAbsoluteName();

    setValid();
}

bool ASTSignaturePrinter::doVisit(ASTNode* node) {
    bool wasValid = _isValid;
    _isValid = false;

    node->onVisit(this);

    _isValid = _isValid && wasValid;
    return _isValid;
}

void ASTSignaturePrinter::setValid() {
    _isValid = true;
}

}

}
