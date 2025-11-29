#include "parser.h"
#include <iostream>

Token current_token;

// --------------------------------------
// utilitários
// --------------------------------------

void advance() {
    current_token = next_token();
}

void error(const std::string& msg) {
    std::cout << "Erro sintático: " << msg
              << " (token '" << current_token.lexema << "')\n";
    exit(1);
}

void expect(TokenType t) {
    if (current_token.tipo != t)
        error("Esperado token diferente");
    advance();
}

// --------------------------------------
// inicialização
// --------------------------------------
void parser_init() {
    advance();
}

// --------------------------------------
// <programa> ::= { <decl> | <comando> }
// --------------------------------------
AST* parse_program() {
    AST* root = create_node(AST_PROGRAMA);

    while (current_token.tipo != TOKEN_EOF) {
        if (is_tipo(current_token.tipo)) {
            root->children.push_back(parse_declaracao());
        } else {
            root->children.push_back(parse_comando());
        }
    }

    return root;
}

// --------------------------------------
// DECLARAÇÕES
// <declaracao> ::= TIPO <lista_decl> ";"
// --------------------------------------
AST* parse_declaracao() {
    std::string tipo = current_token.lexema; // Inteiro, Logico, Caractere
    advance(); // consome TIPO

    AST* decl_node = create_node(AST_DECLARACAO);

    AST* lista = parse_lista_decl(tipo);
    decl_node->children.push_back(lista);

    expect(TOKEN_PONTO_VIRGULA);

    return decl_node;
}

// --------------------------------------
// <lista_decl> ::= <decl> { "," <decl> }
// --------------------------------------
AST* parse_lista_decl(const std::string& tipo) {
    AST* lista = create_node(AST_DECLARACAO);

    lista->children.push_back(parse_decl(tipo));

    while (current_token.tipo == TOKEN_VIRGULA) {
        advance(); // consome vírgula
        lista->children.push_back(parse_decl(tipo));
    }

    return lista;
}

// --------------------------------------
// <decl> ::= ID "<-" <expressao>
// --------------------------------------
AST* parse_decl(const std::string& tipo) {
    if (current_token.tipo != TOKEN_ID)
        error("Esperado identificador na declaração");

    AST* id = create_leaf(AST_EXPR_ID, current_token.lexema);
    advance();

    if (current_token.tipo != TOKEN_ATRIB)
        error("Esperado '<-' na declaração");

    advance();

    AST* expr = parse_expressao();

    AST* atrib = create_node(AST_ATRIBUICAO);
    atrib->value = tipo; // salva tipo declarado
    atrib->children.push_back(id);
    atrib->children.push_back(expr);

    return atrib;
}

// --------------------------------------
// COMANDOS
// --------------------------------------
AST* parse_comando() {
    switch (current_token.tipo) {
        case TOKEN_ENQUANTO: return parse_enquanto();
        case TOKEN_SE:       return parse_se_senao();
        case TOKEN_PARA:     return parse_para();
        case TOKEN_IMPRIMIR: return parse_imprimir();
        case TOKEN_CHAVE_ABRE: return parse_bloco();
        default:
            return parse_atribuicao();
    }
}

// --------------------------------------
// <atribuicao> ::= ID "<-" <expressao> ";"
// --------------------------------------
AST* parse_atribuicao() {
    if (current_token.tipo != TOKEN_ID)
        error("Esperado identificador em atribuicao");

    AST* id = create_leaf(AST_EXPR_ID, current_token.lexema);
    advance();

    if (current_token.tipo != TOKEN_ATRIB)
        error("Esperado '<-'");

    advance();

    AST* expr = parse_expressao();

    AST* atrib = create_node(AST_ATRIBUICAO);
    atrib->children.push_back(id);
    atrib->children.push_back(expr);

    if (current_token.tipo == TOKEN_PONTO_VIRGULA)
        advance();

    return atrib;
}

// --------------------------------------
// IMPRIMIR
// --------------------------------------
AST* parse_imprimir() {
    advance(); // Imprimir

    expect(TOKEN_PAREN_ABRE);

    AST* expr = parse_expressao();

    expect(TOKEN_PAREN_FECHA);
    expect(TOKEN_PONTO_VIRGULA);

    AST* node = create_node(AST_IMPRIMIR);
    node->children.push_back(expr);
    return node;
}

// --------------------------------------
// ENQUANTO
// --------------------------------------
AST* parse_enquanto() {
    advance(); // Enquanto

    AST* cond = parse_expr_logica();
    AST* bloco = parse_bloco();

    AST* node = create_node(AST_ENQUANTO);
    node->children.push_back(cond);
    node->children.push_back(bloco);
    return node;
}

// --------------------------------------
// SE / SENAO
// --------------------------------------
AST* parse_se_senao() {
    advance(); // Se

    AST* cond = parse_expr_logica();
    AST* bloco_se = parse_bloco();

    AST* node = create_node(AST_SE);
    node->children.push_back(cond);
    node->children.push_back(bloco_se);

    if (current_token.tipo == TOKEN_SENAO) {
        advance();
        AST* bloco_s = parse_bloco();
        node->children.push_back(bloco_s);
    }

    return node;
}

// --------------------------------------
// PARA
// --------------------------------------
AST* parse_para() {
    advance(); // Para

    if (current_token.tipo != TOKEN_ID)
        error("Esperado identificador em Para");

    AST* id = create_leaf(AST_EXPR_ID, current_token.lexema);
    advance();

    if (current_token.tipo != TOKEN_EM)
        error("Esperado 'em' em Para");

    advance();

    expect(TOKEN_PAREN_ABRE);

    AST* e1 = parse_expressao();
    expect(TOKEN_VIRGULA);
    AST* e2 = parse_expressao();
    expect(TOKEN_VIRGULA);
    AST* e3 = parse_expressao();

    expect(TOKEN_PAREN_FECHA);

    AST* bloco = parse_bloco();

    AST* node = create_node(AST_PARA);
    node->children.push_back(id);
    node->children.push_back(e1);
    node->children.push_back(e2);
    node->children.push_back(e3);
    node->children.push_back(bloco);

    return node;
}

// --------------------------------------
// BLOCO
// --------------------------------------
AST* parse_bloco() {
    expect(TOKEN_CHAVE_ABRE);

    AST* b = create_node(AST_BLOCO);

    while (current_token.tipo != TOKEN_CHAVE_FECHA) {
        if (is_tipo(current_token.tipo))
            b->children.push_back(parse_declaracao());
        else
            b->children.push_back(parse_comando());
    }

    expect(TOKEN_CHAVE_FECHA);

    return b;
}

// =====================================================================
// EXPRESSÕES COM PRECEDÊNCIA
// expo → mult → soma → relacional → lógica
// =====================================================================

AST* parse_expressao() {
    return parse_expr_logica();
}

// lógica: & ^
AST* parse_expr_logica() {
    AST* left = parse_expr_relacional();

    while (current_token.tipo == TOKEN_AND ||
           current_token.tipo == TOKEN_OR) {

        AST* node = create_node(AST_EXPR_BINOP);
        node->op = current_token.lexema;
        advance();

        node->children.push_back(left);
        node->children.push_back(parse_expr_relacional());
        left = node;
    }

    return left;
}

// relacional
AST* parse_expr_relacional() {
    AST* left = parse_expr_soma();

    while (current_token.tipo == TOKEN_IGUAL ||
           current_token.tipo == TOKEN_DIF ||
           current_token.tipo == TOKEN_MENOR ||
           current_token.tipo == TOKEN_MAIOR ||
           current_token.tipo == TOKEN_MENOR_IGUAL ||
           current_token.tipo == TOKEN_MAIOR_IGUAL) {

        AST* node = create_node(AST_EXPR_BINOP);
        node->op = current_token.lexema;
        advance();

        node->children.push_back(left);
        node->children.push_back(parse_expr_soma());
        left = node;
    }

    return left;
}

// soma / sub
AST* parse_expr_soma() {
    AST* left = parse_expr_mult();

    while (current_token.tipo == TOKEN_SOMA ||
           current_token.tipo == TOKEN_SUB) {

        AST* node = create_node(AST_EXPR_BINOP);
        node->op = current_token.lexema;
        advance();

        node->children.push_back(left);
        node->children.push_back(parse_expr_mult());
        left = node;
    }

    return left;
}

// mult / div / mod
AST* parse_expr_mult() {
    AST* left = parse_expr_expo();

    while (current_token.tipo == TOKEN_MUL ||
           current_token.tipo == TOKEN_DIV ||
           current_token.tipo == TOKEN_MOD) {

        AST* node = create_node(AST_EXPR_BINOP);
        node->op = current_token.lexema;
        advance();

        node->children.push_back(left);
        node->children.push_back(parse_expr_expo());
        left = node;
    }

    return left;
}

// exponenciação (**)
AST* parse_expr_expo() {
    AST* left = parse_termo();

    while (current_token.tipo == TOKEN_EXP) {

        AST* node = create_node(AST_EXPR_BINOP);
        node->op = current_token.lexema;
        advance();

        node->children.push_back(left);
        node->children.push_back(parse_termo());
        left = node;
    }

    return left;
}

// termos
AST* parse_termo() {
    if (current_token.tipo == TOKEN_NUM) {
        AST* n = create_leaf(AST_EXPR_NUM, current_token.lexema);
        advance();
        return n;
    }

    if (current_token.tipo == TOKEN_STRING) {
        AST* s = create_leaf(AST_EXPR_STRING, current_token.lexema);
        advance();
        return s;
    }

    if (current_token.tipo == TOKEN_ID) {
        AST* id = create_leaf(AST_EXPR_ID, current_token.lexema);
        advance();
        return id;
    }

    if (current_token.tipo == TOKEN_PAREN_ABRE) {
        advance();
        AST* e = parse_expressao();
        expect(TOKEN_PAREN_FECHA);
        return e;
    }

    error("Termo inválido");
    return nullptr;
}
