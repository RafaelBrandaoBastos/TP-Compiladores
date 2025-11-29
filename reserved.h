#ifndef RESERVED_H
#define RESERVED_H

#include "hash-table.h"   // agora contendo SymbolInfo, insertSymbol, etc
#include "tokens.h"       // para usar TokenType

// Função para carregar todas as palavras reservadas
void loadReservedWords(struct hashMap* table) {

    // ---------------- PALAVRAS-CHAVE ----------------
    SymbolInfo s;

    s = createSymbol("se",        "reservado", TOKEN_IF, 0);
    insertSymbol(table, s);

    s = createSymbol("senao",     "reservado", TOKEN_ELSE, 0);
    insertSymbol(table, s);

    s = createSymbol("enquanto",  "reservado", TOKEN_WHILE, 0);
    insertSymbol(table, s);

     s = createSymbol("imprimir",      "reservado", TOKEN_PRINT, 0);
    insertSymbol(table, s);

    s = createSymbol("inteiro",   "tipo", TOKEN_TIPO_INTEIRO, 0);
    insertSymbol(table, s);

    s = createSymbol("logico",      "tipo", TOKEN_TIPO_LOGICO, 0);
    insertSymbol(table, s);

    s = createSymbol("caractere", "tipo", TOKEN_TIPO_CARACTERE, 0);
    insertSymbol(table, s);

    s = createSymbol("verdadeiro", "booleano", TOKEN_BOOLEAN, 0);
    storeBoolean(&s, true);
    insertSymbol(table, s);

    s = createSymbol("verdade", "booleano", TOKEN_BOOLEAN, 0);
    storeBoolean(&s, true);
    insertSymbol(table, s);

    s = createSymbol("falso",      "booleano", TOKEN_BOOLEAN, 0);
    storeBoolean(&s, false);
    insertSymbol(table, s);

    s = createSymbol("mentira",      "booleano", TOKEN_BOOLEAN, 0);
    storeBoolean(&s, false);
    insertSymbol(table, s);


    // ---------------- OPERADORES ARITMÉTICOS ----------------
    s = createSymbol("+", "operador", TOKEN_PLUS, 0);
    insertSymbol(table, s);

    s = createSymbol("-", "operador", TOKEN_MINUS, 0);
    insertSymbol(table, s);

    s = createSymbol("*", "operador", TOKEN_MUL, 0);
    insertSymbol(table, s);

    s = createSymbol("/", "operador", TOKEN_DIV, 0);
    insertSymbol(table, s);


    // ---------------- COMPARADORES ----------------
    s = createSymbol(">=", "operador", TOKEN_GE, 0);
    insertSymbol(table, s);

    s = createSymbol("<=", "operador", TOKEN_LE, 0);
    insertSymbol(table, s);

    s = createSymbol("<>", "operador", TOKEN_NE, 0);
    insertSymbol(table, s);

    s = createSymbol(">", "operador", TOKEN_GT, 0);
    insertSymbol(table, s);

    s = createSymbol("<", "operador", TOKEN_LT, 0);
    insertSymbol(table, s);

    s = createSymbol("=", "operador", TOKEN_EQ, 0);
    insertSymbol(table, s);


    // ---------------- ATRIBUIÇÃO ----------------
    s = createSymbol("<-", "operador", TOKEN_ASSIGN, 0);
    insertSymbol(table, s);


    // ---------------- DELIMITADORES ----------------
    s = createSymbol("(", "delimitador", TOKEN_LPAREN, 0);
    insertSymbol(table, s);

    s = createSymbol(")", "delimitador", TOKEN_RPAREN, 0);
    insertSymbol(table, s);

    s = createSymbol("{", "delimitador", TOKEN_LBRACE, 0);
    insertSymbol(table, s);

    s = createSymbol("}", "delimitador", TOKEN_RBRACE, 0);
    insertSymbol(table, s);

    s = createSymbol(";", "delimitador", TOKEN_SEMICOLON, 0);
    insertSymbol(table, s);

    s = createSymbol(",", "delimitador", TOKEN_COMMA, 0);
    insertSymbol(table, s);
}

#endif // RESERVED_H
