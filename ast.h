#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

// Tipos de nós da AST
enum ASTNodeType {
    AST_PROGRAMA,
    AST_DECLARACAO,
    AST_ATRIBUICAO,
    AST_ENQUANTO,
    AST_SE,
    AST_SENAO,
    AST_PARA,
    AST_IMPRIMIR,
    AST_BLOCO,
    AST_EXPR_BINOP,
    AST_EXPR_ID,
    AST_EXPR_NUM,
    AST_EXPR_STRING
};

// Estrutura base para AST
struct AST {
    ASTNodeType type;
    std::string value;           // usado para ID, tipo, e literais
    std::string op;              // operador em exprs binárias
    std::vector<AST*> children;  // filhos
};

// Funções utilitárias
AST* create_node(ASTNodeType type);
AST* create_leaf(ASTNodeType type, const std::string& value);

void print_ast(AST* node, int depth = 0);
void free_ast(AST* node);

#endif
