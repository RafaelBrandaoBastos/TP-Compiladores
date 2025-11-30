// C++ Program to Read a file line by line using getline
#include <fstream>
#include <iostream>
#include <sstream>
#include "hash-table.h"
#include <cstring>
#include "reserved.h"
#include "tokens.h"
#include <vector>
#include <cctype>

using namespace std;

//=============== ANALISADOR LEXICO: LEXICAL ANALYZER ===============
/*
-Lê os caracteres
-Agrupa caracteres formando tokens
-Filtra comentários
-Reconhece números, identificadores, operadores, strings
-Consulta tabela de símbolos somente para saber se é palavra reservada
-Só produz tokens, não interpreta
*/
vector<Token> lexicalAnalyzer(string line, int lineNumber, hashMap *table)
{

    vector<Token> tokens;
    int i = 0;
    int n = line.size();

    while (i < n)
    {

        // Ignorar espaços
        if (isspace(line[i]))
        {
            i++;
            continue;
        }

        // Comentário de 1 linha: $
        if (line[i] == '$' && (i + 1 >= n || line[i + 1] != '$'))
        {
            break;
        }

        // Comentário de múltiplas linhas $$ ... $$
        if (line[i] == '$' && i + 1 < n && line[i + 1] == '$')
        {
            i += 2;
            while (i + 1 < n && !(line[i] == '$' && line[i + 1] == '$'))
                i++;

            if (i + 1 < n)
                i += 2;
            continue;
        }

        // ---------------- STRINGS ----------------
        if (line[i] == '"')
        {
            string text = "";
            i++; // pula abertura

            while (i < n && line[i] != '"')
                text += line[i++];

            i++; // fecha "

            Token tk = {TOKEN_STRING, "", lineNumber};
            strcpy(tk.lexeme, text.c_str());
            tokens.push_back(tk);
            continue;
        }

        // ---------------- IDENTIFICADORES ----------------
        if (isalpha(line[i]))
        {

            string tok = "";
            while (i < n && isalpha(line[i]))
            {
                tok += tolower(line[i]);
                i++;
            }

            SymbolInfo *sym = searchSymbol(table, tok.c_str());

            if (sym)
            {
                // palavra reservada
                Token tk = {(TokenType)sym->tokenType, "", lineNumber};
                strcpy(tk.lexeme, tok.c_str());
                tokens.push_back(tk);
            }
            else
            {
                // identificador novo → inserir tabela
                SymbolInfo s = createSymbol(tok.c_str(), "id", TOKEN_ID, lineNumber);
                insertSymbol(table, s);

                Token tk = {TOKEN_ID, "", lineNumber};
                strcpy(tk.lexeme, tok.c_str());
                tokens.push_back(tk);
            }
            continue;
        }

        // ---------------- NÚMEROS ----------------
        if (isdigit(line[i]))
        {

            string num = "";
            while (i < n && isdigit(line[i]))
                num += line[i++];

            Token tk = {TOKEN_NUM, "", lineNumber};
            strcpy(tk.lexeme, num.c_str());
            tokens.push_back(tk);
            continue;
        }

        // ---------------- OPERADORES DUPLOS ----------------
        if (i + 1 < n)
        {
            string op2 = line.substr(i, 2);

            SymbolInfo *sym = searchSymbol(table, op2.c_str());
            if (sym)
            {
                Token tk = {(TokenType)sym->tokenType, "", lineNumber};
                strcpy(tk.lexeme, op2.c_str());
                tokens.push_back(tk);
                i += 2;
                continue;
            }
        }

        // ---------------- OPERADORES SIMPLES ----------------
        string op1 = line.substr(i, 1);
        SymbolInfo *sym2 = searchSymbol(table, op1.c_str());

        if (sym2)
        {
            Token tk = {(TokenType)sym2->tokenType, "", lineNumber};
            strcpy(tk.lexeme, op1.c_str());
            tokens.push_back(tk);
            i++;
            continue;
        }

        // ---------------- CARACTERE INVÁLIDO ----------------
        Token tk = {TOKEN_INVALID, "", lineNumber};
        tk.lexeme[0] = line[i];
        tk.lexeme[1] = '\0';
        tokens.push_back(tk);

        i++;
    }

    return tokens;
}

//=============== ANALISADOR SINTÁTICO: PARSER ===============
/*
- Recebe a lista de tokens gerados pelo léxico
- Verifica se a sequência de tokens segue as regras da gramática
- Detecta erros de sintaxe (token inesperado, falta de ';', etc.)
- Organiza os tokens
- Garante que a estrutura do programa é válida
*/

vector<Token> tokens;
int current = 0;

// ----------------- Funções auxiliares -----------------

Token peek()
{
    return tokens[current];
}

Token previous()
{
    return tokens[current - 1];
}

bool isAtEnd()
{
    return peek().type == TOKEN_EOF;
}

Token advance()
{
    if (!isAtEnd())
        current++;
    return previous();
}

bool check(TokenType type)
{
    if (isAtEnd())
        return false;
    return peek().type == type;
}

bool match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

void error(const string &msg)
{
    cout << "ERRO SINTÁTICO na linha "
         << peek().line << ": " << msg
         << " (token: " << peek().lexeme << ")\n";
    exit(1);
}

void consume(TokenType type, const string &msg)
{
    if (check(type))
        advance();
    else
        error(msg);
}

// ---------------- EXPRESSÕES -----------------

void expression(); // forward

// fator primário: número, id, string, (expr)
void primary()
{
    if (match(TOKEN_NUM))
        return;
    if (match(TOKEN_ID))
        return;
    if (match(TOKEN_STRING))
        return;
    if (match(TOKEN_BOOLEAN))
        return;

    if (match(TOKEN_LPAREN))
    {
        expression();
        consume(TOKEN_RPAREN, "Esperado ')'");
        return;
    }

    error("Expressão primária inválida");
}

// exponenciação **
void exponent()
{
    primary();
    while (match(TOKEN_EXP))
    {
        primary();
    }
}

// *, /, %
void term()
{
    exponent();
    while (match(TOKEN_MUL) || match(TOKEN_DIV) || match(TOKEN_MOD))
    {
        exponent();
    }
}

// +, -
void arithmetic()
{
    term();
    while (match(TOKEN_PLUS) || match(TOKEN_MINUS))
    {
        term();
    }
}

// comparações: <, >, <=, >=, =, <>
void comparison()
{
    arithmetic();
    while (match(TOKEN_LT) || match(TOKEN_GT) ||
           match(TOKEN_LE) || match(TOKEN_GE) ||
           match(TOKEN_EQ) || match(TOKEN_NE))
    {
        arithmetic();
    }
}

// lógico: &, ^
void logic()
{
    comparison();
    while (match(TOKEN_AND) || match(TOKEN_OR))
    {
        comparison();
    }
}

void expression()
{
    logic();
}

// ---------------- DECLARAÇÕES -----------------

void declarationList()
{
    // Tipos: inteiro, logico, caractere
    advance(); // consome o tipo

    consume(TOKEN_ID, "Esperado nome da variável");

    // pode ter atribuicoes:  x <- 10
    if (match(TOKEN_ASSIGN))
    {
        expression();
    }

    // múltiplas declarações:  inteiro a <- 1, b <- 2;
    while (match(TOKEN_COMMA))
    {
        consume(TOKEN_ID, "Esperado identificador");

        if (match(TOKEN_ASSIGN))
        {
            expression();
        }
    }

    consume(TOKEN_SEMICOLON, "Esperado ';' ao final da declaração");
}

//---------------- COMANDOS -----------------
void statement();

void block()
{
    consume(TOKEN_LBRACE, "Esperado '{' para iniciar bloco");
    while (!check(TOKEN_RBRACE) && !isAtEnd())
        statement();
    consume(TOKEN_RBRACE, "Esperado '}' ao finalizar bloco");
}

// ---------- atribuição ----------
void assignment()
{
    consume(TOKEN_ID, "Esperado identificador");
    consume(TOKEN_ASSIGN, "Esperado '<-'");
    expression();
    consume(TOKEN_SEMICOLON, "Esperado ';'");
}

// ---------- imprimir ----------
void printStmt()
{
    consume(TOKEN_LPAREN, "Esperado '(' após imprimir");
    expression();
    consume(TOKEN_RPAREN, "Esperado ')'");
    consume(TOKEN_SEMICOLON, "Esperado ';'");
}

// ---------- se / senao ----------
void ifStmt()
{
    consume(TOKEN_LPAREN, "Esperado '(' após se");
    expression();
    consume(TOKEN_RPAREN, "Esperado ')'");

    block();

    if (match(TOKEN_ELSE))
    {
        block();
    }
}

// ---------- enquanto ----------
void whileStmt()
{
    consume(TOKEN_LPAREN, "Esperado '(' após enquanto");
    expression();
    consume(TOKEN_RPAREN, "Esperado ')'");
    block();
}

// ---------- para ----------
void forStmt()
{
    consume(TOKEN_ID, "Esperado variável de iteração");
    consume(TOKEN_EM, "Esperado 'em'");
    consume(TOKEN_LPAREN, "Esperado '('");

    expression(); // início
    consume(TOKEN_COMMA, "Esperado ','");

    expression(); // fim
    consume(TOKEN_COMMA, "Esperado ','");

    expression(); // passo
    consume(TOKEN_RPAREN, "Esperado ')'");

    block();
}

// ---------- statement -----------
void statement()
{

    if (check(TOKEN_TIPO_INTEIRO) || check(TOKEN_TIPO_LOGICO) ||
        check(TOKEN_TIPO_CARACTERE))
    {
        declarationList();
        return;
    }

    if (check(TOKEN_ID))
    {
        assignment();
        return;
    }

    if (match(TOKEN_PRINT))
    {
        printStmt();
        return;
    }

    if (match(TOKEN_IF))
    {
        ifStmt();
        return;
    }

    if (match(TOKEN_WHILE))
    {
        whileStmt();
        return;
    }

    if (match(TOKEN_FOR))
    {
        forStmt();
        return;
    }

    error("Comando inválido");
}

//--------------- FUNÇÃO PRINCIPAL DO PARSER -----------------
void Parser(vector<Token> listaTokens)
{
    tokens = listaTokens;
    current = 0;

    while (!isAtEnd())
    {
        statement();
    }

    cout << "Analise sintatica concluida com sucesso!\n";
}

int main()
{

    // 1. Criar e preencher a tabela hash
    struct hashMap SymbolTable;
    initializeHashMap(&SymbolTable);

    // 2.Carregar palavras-chave (aquela função nova!)
    loadReservedWords(&SymbolTable);

    // 3. Abrir arquivo de entrada
    ifstream input("input.txt");
    if (!input.is_open())
    {
        cout << "Erro: nao foi possivel abrir input.txt\n";
        return 1;
    }
    else
    {
        cout << "Arquivo input.txt aberto com sucesso\n";
    }

    // 4. Cria o arquivo de saída dos tokens
    ofstream output("tokens.txt");
    if (!output.is_open())
    {
        cout << "Erro: nao foi possivel criar tokens.txt\n";
        return 1;
    }

    // 5. Ler linhas e gerar TODOS os tokens do arquivo
    string line;
    int lineNumber = 1;

    vector<Token> ALL_TOKENS; // <-- VETOR ÚNICO PARA O ARQUIVO INTEIRO

    while (getline(input, line))
    {
        vector<Token> tokensLine = lexicalAnalyzer(line, lineNumber, &SymbolTable);
        // Salva tokens da linha no vetor geral
        for (const Token &t : tokensLine)
        {
            ALL_TOKENS.push_back(t);
            // ainda grava no arquivo de depuração tokens.txt
            output << tokenTypeName(t.type)
                   << "  \"" << t.lexeme << "\""
                   << "   linha " << t.line << "\n";
        }

        lineNumber++;
    }
    cout << "Análise léxica concluída. Tokens salvos em tokens.txt.\n";

    // 6. Token EOF
    Token eof;
    eof.type = TOKEN_EOF;
    eof.lexeme[0] = '\0';
    eof.line = lineNumber;
    ALL_TOKENS.push_back(eof);
    output << "EOF\n";

    // 7. Chama o Parser com TODOS os tokens
    Parser(ALL_TOKENS);
    cout << "Análise sintática concluída com sucesso!\n";

    // 8. Fecha arquivos
    input.close();
    output.close();

    return 0;
}
