#include "ast.h"
#include <iostream>

AST* create_node(ASTNodeType type) {
    AST* n = new AST();
    n->type = type;
    return n;
}

AST* create_leaf(ASTNodeType type, const std::string& value) {
    AST* n = new AST();
    n->type = type;
    n->value = value;
    return n;
}

void print_ast(AST* node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++)
        std::cout << "  ";

    std::cout << "- ";

    switch (node->type) {
        case AST_PROGRAMA:   std::cout << "PROGRAMA"; break;
        case AST_DECLARACAO: std::cout << "DECLARACAO"; break;
        case AST_ATRIBUICAO: std::cout << "ATRIBUICAO"; break;
        case AST_ENQUANTO:   std::cout << "ENQUANTO"; break;
        case AST_SE:         std::cout << "SE"; break;
        case AST_SENAO:      std::cout << "SENAO"; break;
        case AST_PARA:       std::cout << "PARA"; break;
        case AST_IMPRIMIR:   std::cout << "IMPRIMIR"; break;
        case AST_BLOCO:      std::cout << "BLOCO"; break;
        case AST_EXPR_BINOP: std::cout << "BINOP(" << node->op << ")"; break;
        case AST_EXPR_ID:    std::cout << "ID(" << node->value << ")"; break;
        case AST_EXPR_NUM:   std::cout << "NUM(" << node->value << ")"; break;
        case AST_EXPR_STRING:std::cout << "STRING(" << node->value << ")"; break;
    }

    std::cout << "\n";

    for (auto c : node->children)
        print_ast(c, depth + 1);
}

void free_ast(AST* node) {
    if (!node) return;
    for (auto c : node->children)
        free_ast(c);
    delete node;
}
