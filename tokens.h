#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>

/*
    Lista completa de tokens de CF
*/
typedef enum {

    // Tipos
    TOKEN_TIPO_INTEIRO,
    TOKEN_TIPO_LOGICO,
    TOKEN_TIPO_CARACTERE,

    // comandos
    TOKEN_IF,         // Se
    TOKEN_ELSE,       // Senao
    TOKEN_WHILE,      // Enquanto
    TOKEN_FOR,        // Para
    TOKEN_PRINT,      // Imprimir

    // Identificadores e literais
    TOKEN_ID,
    TOKEN_NUM,
    TOKEN_STRING,
    TOKEN_BOOLEAN,    // Verdade / Mentira

    // Operadores aritméticos
    TOKEN_PLUS,       // +
    TOKEN_MINUS,      // -
    TOKEN_MUL,        // *
    TOKEN_DIV,        // /
    TOKEN_MOD,        // %
    TOKEN_EXP,        // **

    // Operadores lógicos
    TOKEN_EQ,         // =
    TOKEN_NE,         // <>
    TOKEN_GT,         // >
    TOKEN_LT,         // <
    TOKEN_GE,         // >=
    TOKEN_LE,         // <=
    TOKEN_AND,        // &
    TOKEN_OR,         // ^

    // Atribuição (CF usa "<-")
    TOKEN_ASSIGN,     // <-

    // Símbolos
    TOKEN_LBRACE,     // {
    TOKEN_RBRACE,     // }
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN,     // )
    TOKEN_SEMICOLON,  // ;
    TOKEN_COMMA,      // ,

    // Fim de arquivo
    TOKEN_EOF,

    // Token inválido
    TOKEN_INVALID

} TokenType;

/*
    Estrutura de Token retornada pelo Analisador Léxico
*/
typedef struct {
    TokenType type;         // categoria
    char lexeme[128];       // texto original do token
    int line;               // linha do código-fonte
} Token;





/*
    Função para debug
*/
static inline const char* tokenTypeName(TokenType type) {
    switch(type) {

        case TOKEN_TIPO_INTEIRO: return "<TIPO_INTEIRO>";
        case TOKEN_TIPO_LOGICO: return "<TIPO_LOGICO>";
        case TOKEN_TIPO_CARACTERE: return "<TIPO_CARACTERE>";

        case TOKEN_IF: return "<IF>";
        case TOKEN_ELSE: return "<ELSE>";
        case TOKEN_WHILE: return "<WHILE>";
        case TOKEN_FOR: return "<FOR>";
        case TOKEN_PRINT: return "<PRINT>";

        case TOKEN_ID:      return "<ID>";
        case TOKEN_NUM:     return "<NUM>";
        case TOKEN_STRING:  return "<STRING>";
        case TOKEN_BOOLEAN: return "<BOOLEAN>";

        case TOKEN_PLUS:  return "<PLUS>";
        case TOKEN_MINUS: return "<MINUS>";
        case TOKEN_MUL:   return "<MUL>";
        case TOKEN_DIV:   return "<DIV>";
        case TOKEN_MOD:   return "<MOD>";
        case TOKEN_EXP:   return "<EXP>";

        case TOKEN_EQ: return "<EQ>";
        case TOKEN_NE: return "<NE>";
        case TOKEN_GT: return "<GT>";
        case TOKEN_LT: return "<LT>";
        case TOKEN_GE: return "<GE>";
        case TOKEN_LE: return "<LE>";
        case TOKEN_AND:return "<AND>";
        case TOKEN_OR: return "<OR>";

        case TOKEN_ASSIGN: return "<ASSIGN>";

        case TOKEN_LBRACE:    return "<LBRACE>";
        case TOKEN_RBRACE:    return "<RBRACE>";
        case TOKEN_LPAREN:    return "<LPAREN>";
        case TOKEN_RPAREN:    return "<RPAREN>";
        case TOKEN_SEMICOLON: return "<SEMICOLON>";
        case TOKEN_COMMA:     return "<COMMA>";

        case TOKEN_EOF:     return "<EOF>";
        case TOKEN_INVALID: return "<INVALID>";

        default: return "<UNKNOWN_TOKEN>";
    }
}

#endif
