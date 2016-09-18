//
//  UsageAnalysis.h
//  SFSL
//
//  Created by Romain Beguet on 01.08.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__UsageAnalysis__
#define __SFSL__UsageAnalysis__

#include <iostream>
#include <map>
#include <vector>
#include "../AST/Visitors/ASTImplicitVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class UsageAnalysis : public ASTImplicitVisitor {
public:

    UsageAnalysis(CompCtx_Ptr& ctx);
    virtual ~UsageAnalysis();

    virtual void visit(Program* prog) override;

    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* def) override;

    virtual void visit(FunctionCreation* func) override;
    virtual void visit(Instantiation* inst) override;

private:

    struct InstantiationInfo {
        InstantiationInfo(Instantiation* instNode, ASTNode* instUnit);

        Instantiation* instNode;
        ASTNode* instUnit;
    };

    std::map<sym::VariableSymbol*, std::vector<Identifier*>> _undeclaredVars;
    TypeExpression* _nextInstantiatedExpression;

    ASTNode* _currentUnit;
    std::map<ClassDecl*, std::vector<InstantiationInfo>> _classInstantiationsToUnits;
};

}

}

#endif
