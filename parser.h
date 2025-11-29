#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include "ast.h"

// token atual
extern Token current_token;

// inicialização
void parser_init();

// regra principal
AST* parse_program();

// regras da gramática
AST* parse_declaracao();
AST* parse_lista_decl(const std::string& tipo);
AST* parse_decl(const std::string& tipo);
AST* parse_comando();
AST* parse_atribuicao();
AST* parse_imprimir();
AST* parse_enquanto();
AST* parse_se_senao();
AST* parse_para();
AST* parse_bloco();

// expressões com precedência
AST* parse_expressao();
AST* parse_expr_logica();
AST* parse_expr_relacional();
AST* parse_expr_soma();
AST* parse_expr_mult();
AST* parse_expr_expo();
AST* parse_termo();

#endif
