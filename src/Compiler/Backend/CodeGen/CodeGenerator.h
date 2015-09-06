//
//  CodeGenerator.h
//  SFSL
//
//  Created by Romain Beguet on 21.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CodeGenerator__
#define __SFSL__CodeGenerator__

#include <iostream>
#include <set>
#include "../../Frontend/AST/Visitors/ASTVisitor.h"
#include "../../Frontend/AST/Symbols/SymbolResolver.h"
#include "CodeGenOutput.h"

namespace sfsl {

namespace out {

using namespace ast;

template<typename T>
/**
 * @brief Base class for visitors that generate code from the AST
 */
class CodeGenerator : public ASTVisitor {
public:

    CodeGenerator(CompCtx_Ptr& ctx, CodeGenOutput<T>& out) : ASTVisitor(ctx), _mngr(ctx->memoryManager()), _out(out) {}
    virtual ~CodeGenerator() {}

    virtual void visit(ASTNode*) override = 0;

protected:

    common::AbstractMemoryManager& _mngr;
    CodeGenOutput<T>& _out;
};

}

}

#endif
