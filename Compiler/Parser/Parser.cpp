//
//  Parser.cpp
//  SFSL
//
//  Created by Romain Beguet on 16.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Parser.h"

namespace sfsl {

using namespace tok;
using namespace ast;

Parser::Parser(common::CompilationContext &ctx, lex::Lexer &lexer) : _ctx(ctx), _lex(lexer) {

}

ASTNode* Parser::parse() {

}



}
