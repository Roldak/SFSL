//
//  ASTSignaturePrinter.h
//  SFSL
//
//  Created by Romain Beguet on 08.09.16.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTSignaturePrinter__
#define __SFSL__ASTSignaturePrinter__

#include <iostream>
#include <set>
#include "ASTExplicitVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class ASTSignaturePrinter : protected ASTExplicitVisitor {
public:
    virtual ~ASTSignaturePrinter();

    static bool printSignatureOf(ASTNode* node, std::ostream& ostream, CompCtx_Ptr& ctx);
    static std::string getSignatureOf(ASTNode* node, bool& isValid, CompCtx_Ptr& ctx);

protected:

    ASTSignaturePrinter(CompCtx_Ptr& ctx, std::ostream& ostream);

    virtual void visit(FunctionTypeDecl* ftdecl) override;
    virtual void visit(TypeConstructorCall* tcall) override;

    virtual void visit(TypeMemberAccess* mac) override;
    virtual void visit(TypeIdentifier* ident) override;

    bool doVisit(ASTNode* node);
    void setValid();

    bool _isValid;

    std::ostream& _ostream;
};

}

}

#endif
