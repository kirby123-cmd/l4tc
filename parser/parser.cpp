#include "./parser.hpp"

namespace parser {
  std::shared_ptr<ASTTypeSpec> parse_type_spec(Token **next, Error &err) {
    Token *t;
    if (
      (t = expect_token_with_type(next, err, KwNum)) ||
      (t = expect_token_with_type(next, err, KwStr))
    ) {
      return std::make_shared<ASTTypeSpec>(t);
    }
    return nullptr;
  }

  std::shared_ptr<ASTTypeSpec> parse_declaration_spec(Token **next, Error &err) {
    return parse_type_spec(next, err);
  }

  std::shared_ptr<ASTExpr> parse_expr(Token **next, Error &err);
  std::shared_ptr<ASTExpr> parse_primary_expr(Token **next, Error &err) {
    Token *t;
    if (expect_token_with_str(next, err, "(")) {
      std::shared_ptr<ASTPrimaryExpr> ret = std::make_shared<ASTPrimaryExpr>();
      if (!(ret->expr = parse_expr(next, err))) return nullptr;
      if (expect_token_with_str(next, err, ")")) return ret;
    } else if (
      (t = expect_token_with_type(next, err, NumberConstant)) ||
      (t = expect_token_with_type(next, err, Ident))
    ) {
      return std::make_shared<ASTSimpleExpr>(t);
    }
    return nullptr;
  }

  std::shared_ptr<ASTExpr> parse_assign_expr(Token **next, Error &err);
  std::shared_ptr<ASTExpr> parse_postfix_expr(Token **next, Error &err) {
  // function-call
  // array
  // increment
  // decrement
  // ->
  // .
    std::shared_ptr<ASTExpr> primary = parse_primary_expr(next, err);
    if (!primary) return nullptr;
    // if "(" is not here, it is not function-call
    // but it is correct primary expr
    if (!expect_token_with_str(next, err, "(")) return primary;
    // now it is function-call-expr
    std::shared_ptr<ASTFuncCallExpr> ret = std::make_shared<ASTFuncCallExpr>();
    ret->primary = primary;

    std::shared_ptr<ASTExpr> arg;
    while (!expect_token_with_str(next, err, ")")) {
      if (!(arg = parse_assign_expr(next, err))) return nullptr;
      ret->args.push_back(arg);
      if (expect_token_with_str(next, err, ",")) continue;
      // end
      if (!expect_token_with_str(next, err, ")")) return nullptr;
      break;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_unary_expr(Token **next, Error &err) {
    return parse_postfix_expr(next, err);
  }

  std::shared_ptr<ASTExpr> parse_multiplicative_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_unary_expr(next, err);
    if (!ret) return nullptr;
    Token *t;
    while (
      (t = expect_token_with_str(next, err, "*")) ||
      (t = expect_token_with_str(next, err, "/")) ||
      (t = expect_token_with_str(next, err, "%"))
    ) {
      left = ret;
      ret = std::make_shared<ASTMultiplicativeExpr>(t);
      ret->left = left;
      if (!(ret->right = parse_unary_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_additive_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_multiplicative_expr(next, err);
    if (!ret) return nullptr;
    Token *t;
    while (
      (t = expect_token_with_str(next, err, "+")) ||
      (t = expect_token_with_str(next, err, "-"))
    ) {
      left = ret;
      ret = std::make_shared<ASTAdditiveExpr>(t);
      ret->left = left;
      if (!(ret->right = parse_multiplicative_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_shift_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_additive_expr(next, err);
    if (!ret) return nullptr;
    Token *t;
    while (
      (t = expect_token_with_str(next, err, "<<")) ||
      (t = expect_token_with_str(next, err, ">>"))
    ) {
      left = ret;
      ret = std::make_shared<ASTShiftExpr>(t);
      ret->left = left;
      if (!(ret->right = parse_additive_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_relational_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_shift_expr(next, err);
    if (!ret) return nullptr;
    Token *t;
    while (
      (t = expect_token_with_str(next, err, "<")) ||
      (t = expect_token_with_str(next, err, ">")) ||
      (t = expect_token_with_str(next, err, "<=")) ||
      (t = expect_token_with_str(next, err, ">="))
    ) {
      left = ret;
      ret = std::make_shared<ASTRelationalExpr>(t);
      ret->left = left;
      if (!(ret->right = parse_shift_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_equality_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_relational_expr(next, err);
    if (!ret) return nullptr;
    Token *t;
    while (
      (t = expect_token_with_str(next, err, "==")) ||
      (t = expect_token_with_str(next, err, "!="))
    ) {
      left = ret;
      ret = std::make_shared<ASTEqualityExpr>(t);
      ret->left = left;
      if (!(ret->right = parse_relational_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_bitwise_and_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_equality_expr(next, err);
    if (!ret) return nullptr;
    while (expect_token_with_str(next, err, "&")) {
      left = ret;
      ret = std::make_shared<ASTBitwiseAndExpr>();
      ret->left = left;
      if (!(ret->right = parse_equality_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_bitwise_xor_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_bitwise_and_expr(next, err);
    if (!ret) return nullptr;
    while (expect_token_with_str(next, err, "^")) {
      left = ret;
      ret = std::make_shared<ASTBitwiseXorExpr>();
      ret->left = left;
      if (!(ret->right = parse_bitwise_and_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_bitwise_or_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_bitwise_xor_expr(next, err);
    if (!ret) return nullptr;
    while (expect_token_with_str(next, err, "|")) {
      left = ret;
      ret = std::make_shared<ASTBitwiseOrExpr>();
      ret->left = left;
      if (!(ret->right = parse_bitwise_xor_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_logical_and_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_bitwise_or_expr(next, err);
    if (!ret) return nullptr;
    while (expect_token_with_str(next, err, "&&")) {
      left = ret;
      ret = std::make_shared<ASTLogicalAndExpr>();
      ret->left = left;
      if (!(ret->right = parse_bitwise_or_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_logical_or_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left, ret = parse_logical_and_expr(next, err);
    if (!ret) return nullptr;
    while (expect_token_with_str(next, err, "||")) {
      left = ret;
      ret = std::make_shared<ASTLogicalOrExpr>();
      ret->left = left;
      if (!(ret->right = parse_logical_and_expr(next, err))) return nullptr;
    }
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_assign_expr(Token **next, Error &err) {
    std::shared_ptr<ASTExpr> left = parse_logical_or_expr(next, err);
    // parse error
    if (!left) return nullptr;
    // if it is not unary-expr, it can't be left of assignment-expr
    // but it is correct expr
    if (!is_unary_expr(left)) return left;
    // if ":" is not here, it is not assignment-expr
    // but it is correct expr
    if (!expect_token_with_str(next, err, ":")) return left;
    // now it is assignment-expr
    std::shared_ptr<ASTAssignExpr> ret = std::make_shared<ASTAssignExpr>();
    ret->left = left;
    if (!(ret->right = parse_assign_expr(next, err))) return nullptr;
    return ret;
  }

  std::shared_ptr<ASTExpr> parse_expr(Token **next, Error &err) {
    return parse_assign_expr(next, err);
  }

  std::shared_ptr<ASTExprStmt> parse_expr_stmt(Token **next, Error &err) {
    std::shared_ptr<ASTExprStmt> ret = std::make_shared<ASTExprStmt>();
    if (!(ret->expr = parse_expr(next, err))) return nullptr;
    if (!expect_token_with_str(next, err, "\n")) return nullptr;
    return ret;
  }

  std::shared_ptr<ASTDeclarator> parse_declarator(Token **next, Error &err) {
    Token *t = expect_token_with_type(next, err, Ident);
    if (!t) return nullptr;
    return std::make_shared<ASTDeclarator>(t);
  }

  std::shared_ptr<ASTDeclaration> parse_declaration(Token **next, Error &err) {
    std::shared_ptr<ASTDeclaration> ret = std::make_shared<ASTDeclaration>();
    if (!(ret->declaration_spec = parse_declaration_spec(next, err))) return nullptr;

    std::shared_ptr<ASTDeclarator> declarator;
    while ((declarator = parse_declarator(next, err))) {
      ret->declarators.push_back(declarator);
      if (expect_token_with_str(next, err, ",")) continue;
      // declaration end
      if (expect_token_with_str(next, err, "\n")) return ret;
      break;
    }
    return nullptr;
  }

  std::shared_ptr<ASTSimpleDeclaration> parse_simple_declaration(Token **next, Error &err) {
  // type-specifier declarator
    std::shared_ptr<ASTSimpleDeclaration> ret = std::make_shared<ASTSimpleDeclaration>();
    if (!(ret->type_spec = parse_type_spec(next, err))) return nullptr;
    if (!(ret->declarator = parse_declarator(next, err))) return nullptr;
    return ret;
  }

  std::shared_ptr<ASTCompoundStmt> parse_comp_stmt(Token **next, Error &err, int indents);

  std::shared_ptr<ASTElseStmt> parse_else_stmt(Token **next, Error &err, int indents) {
    std::shared_ptr<ASTElseStmt> ret = std::make_shared<ASTElseStmt>();
    if (expect_token_with_type(next, err, KwElif)) {
      if (!(ret->cond = parse_expr(next, err))) return nullptr;
    } else if (!expect_token_with_type(next, err, KwElse)) {
      return nullptr;
    }
    if (!expect_token_with_str(next, err, "\n")) return nullptr;
    if (!(ret->true_stmt = parse_comp_stmt(next, err, indents + 2))) return nullptr;
    Token *saved_token = *next;
    if (!consume_token_with_indents(next, indents) ||
        !(ret->false_stmt = parse_else_stmt(next, err, indents))
    ) {
      *next = saved_token;
      return ret;
    }
    return ret;
  }

  std::shared_ptr<AST> parse_stmt(Token **next, Error &err, int indents) {
    if (expect_token_with_type(next, err, KwBreak)) {
      if (!expect_token_with_str(next, err, "\n")) return nullptr;
      return std::make_shared<ASTBreakStmt>();
    }
    if (expect_token_with_type(next, err, KwContinue)) {
      if (!expect_token_with_str(next, err, "\n")) return nullptr;
      return std::make_shared<ASTContinueStmt>();
    }
    if (expect_token_with_type(next, err, KwReturn)) {
      std::shared_ptr<ASTReturnStmt> ret = std::make_shared<ASTReturnStmt>();
      if (!(ret->expr = parse_expr(next, err))) return nullptr;
      if (!expect_token_with_str(next, err, "\n")) return nullptr;
      return ret;
    }
    if (expect_token_with_type(next, err, KwIf)) {
      std::shared_ptr<ASTIfStmt> ret = std::make_shared<ASTIfStmt>();
      if (!(ret->cond = parse_expr(next, err))) return nullptr;
      if (!expect_token_with_str(next, err, "\n")) return nullptr;
      if (!(ret->true_stmt = parse_comp_stmt(next, err, indents + 2))) return nullptr;
      Token *saved_token = *next;
      if (!consume_token_with_indents(next, indents)) {
        *next = saved_token;
        return ret;
      }
      if (!(ret->false_stmt = parse_else_stmt(next, err, indents))) {
        *next = saved_token;
        return ret;
      }
      return ret;
    }
    return parse_expr_stmt(next, err);
    // TODO: loop
  }

  std::shared_ptr<ASTCompoundStmt> parse_comp_stmt(Token **next, Error &err, int indents) {
    std::shared_ptr<ASTCompoundStmt> ret = std::make_shared<ASTCompoundStmt>();
    std::shared_ptr<AST> item;
    while (1) {
      if (!consume_token_with_indents(next, indents)) {
        if (!*next || (*next)->sv.front() != ' ' || (int)(*next)->sv.length() < indents) {
          // compound-stmt end
          return ret;
        } else {
        // inner compound-stmt
          if (!(item = parse_comp_stmt(next, err, indents + 2))) break;
        }
      } else if (
        !(item = parse_declaration(next, err)) &&
        !(item = parse_stmt(next, err, indents))
      ) {
        break;
      }
      ret->items.push_back(item);
      // if (typeid(*item) == typeid(ASTElseStmt)) {
      //   if (!ret->items.size()) 
      // }
      // if (
      //   typeid(*ret->items.back()) == typeid(ASTIfStmt) &&
      //   typeid(*item) == typeid(ASTElseStmt)
      // ) {
      //   std::dynamic_pointer_cast<ASTIfStmt>(ret->items.back())
      //     ->false_stmt = std::dynamic_pointer_cast<ASTElseStmt>(item);
      // } else if (
      //   typeid(*ret->items.back()) == typeid(ASTElseStmt) &&
      //   typeid(*item) == typeid(ASTElseStmt)
      // ) {
      //   std::dynamic_pointer_cast<ASTElseStmt>(ret->items.back())
      //     ->false_stmt = std::dynamic_pointer_cast<ASTElseStmt>(item);
      // } else {
      //   ret->items.push_back(item);
      // }
    }
    return nullptr;
  }

  std::shared_ptr<ASTFuncDeclarator> parse_func_declarator(Token **next, Error &err) {
  // declarator(declaration, ...)
    std::shared_ptr<ASTFuncDeclarator> ret = std::make_shared<ASTFuncDeclarator>();
    if (!(ret->declarator = parse_declarator(next, err))) return nullptr;
    if (!expect_token_with_str(next, err, "(")) return nullptr;

    std::shared_ptr<ASTSimpleDeclaration> declaration;
    while (!expect_token_with_str(next, err, ")")) {
      if (!(declaration = parse_simple_declaration(next, err))) return nullptr;
      ret->args.push_back(declaration);
      if (expect_token_with_str(next, err, ",")) continue;
      // end
      if (!expect_token_with_str(next, err, ")")) return nullptr;
      break;
    }
    return ret;
  }

  std::shared_ptr<ASTFuncDeclaration> parse_func_declaration(Token **next, Error &err) {
  // func-declarator -> type
    std::shared_ptr<ASTFuncDeclaration> ret = std::make_shared<ASTFuncDeclaration>();
    if (!(ret->declarator = parse_func_declarator(next, err))) return nullptr;
    // token "->" should be here
    if (!expect_token_with_str(next, err, "->")) return nullptr;
    if (!(ret->type_spec = parse_type_spec(next, err))) return nullptr;
    // token LF should be here
    if (!expect_token_with_str(next, err, "\n")) return nullptr;
    return ret;
  }

  std::shared_ptr<ASTFuncDef> parse_func_def(Token **next, Error &err) {
  // func-declaration compound-stmt
    if (!expect_token_with_type(next, err, KwFunc)) return nullptr;
    std::shared_ptr<ASTFuncDef> ret = std::make_shared<ASTFuncDef>();
    if (!(ret->declaration = parse_func_declaration(next, err))) return nullptr;
    if (!(ret->body = parse_comp_stmt(next, err, 2))) return nullptr;
    return ret;
  }

  std::shared_ptr<ASTExternalDeclaration> parse_external_declaration(Token **next, Error &err) {
    std::shared_ptr<ASTExternalDeclaration> ret = std::make_shared<ASTExternalDeclaration>();
    if (!(ret->declaration_spec = parse_declaration_spec(next, err))) return nullptr;

    std::shared_ptr<ASTDeclarator> declarator;
    while ((declarator = parse_declarator(next, err))) {
      ret->declarators.push_back(declarator);
      if (expect_token_with_str(next, err, ",")) continue;
      // declaration end
      if (expect_token_with_str(next, err, "\n")) return ret;
      break;
    }
    return nullptr;
  }

  std::shared_ptr<ASTTranslationUnit> parse_translation_unit(Token **next, Error &err) {
    std::shared_ptr<ASTTranslationUnit> ret = std::make_shared<ASTTranslationUnit>();

    std::shared_ptr<AST> external_declaration;
    while (*next) {
      if ((*next)->sv == "func") external_declaration = parse_func_def(next, err);
      else external_declaration = parse_external_declaration(next, err);
      if (!external_declaration) return nullptr;
      ret->external_declarations.push_back(external_declaration);
    }
    return ret;
  }

  void init_parser(Token **head_token) {
    // *head_token can be NULL
    Token *next = *head_token;
    Token *t, *bef;
    while (next) {
      // if next token is Delimiter, remove it and continue
      t = consume_token_with_type(&next, Delimiter);    
      if (!t) break;
      delete t;
    }
    // set new head_token
    *head_token = next;
    // remove first LF punctuator
    if (*head_token) {
      t = consume_token_with_str(head_token, "\n");
      if (!t) delete t;
    }
    // *head_token can be NULL
    next = *head_token;
    while (next) {
      bef = next;
      assert(bef->type != Delimiter);
      next = bef->next;
      while (next) {
        // if next token is Delimiter, remove it and continue
        t = consume_token_with_type(&next, Delimiter);
        if (!t) break;
        delete t;
      }
      bef->next = next;
    }
  }

  std::shared_ptr<ASTTranslationUnit> parse(Token **head_token, Error &err) {
    init_parser(head_token);
    Token *next = *head_token;
    return parse_translation_unit(&next, err);
  }
}