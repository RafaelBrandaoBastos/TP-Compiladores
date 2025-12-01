// ------------------------ semantic.h / semantic.cpp (colocar no mesmo arquivo) ------------------------
// Se tiverem erros no vscode são apenas bugs do Intellisense que não interferem na compilação
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>

using namespace std;

// Tipos para análise semântica
enum VarType
{
    TYPE_INT,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_ERROR,
    TYPE_VOID
};

// --- Helpers para converter entre string-type e VarType ---
VarType typeFromString(const char *s)
{
    if (strcmp(s, "inteiro") == 0)
        return TYPE_INT;
    if (strcmp(s, "logico") == 0)
        return TYPE_BOOL;
    if (strcmp(s, "caractere") == 0)
        return TYPE_CHAR;
    return TYPE_ERROR;
}

const char *typeName(VarType t)
{
    switch (t)
    {
    case TYPE_INT:
        return "inteiro";
    case TYPE_BOOL:
        return "logico";
    case TYPE_CHAR:
        return "caractere";
    case TYPE_STRING:
        return "string";
    default:
        return "erro";
    }
}

// --- PILHA DE ESCOPOs (cada escopo é um hashMap) ---
vector<hashMap *> scopeStack;      // topo = último elemento
hashMap *globalReserved = nullptr; // aponta para a tabela que contém palavras reservadas

void pushScope()
{
    hashMap *m = (hashMap *)malloc(sizeof(hashMap));
    initializeHashMap(m);
    scopeStack.push_back(m);
}

void popScope()
{
    if (scopeStack.empty())
        return;
    hashMap *top = scopeStack.back();
    // NOTE: não libera a lista internamente (podemos deixar para simplificar)
    // mas liberar memória seria ideal — aqui assumimos execução curta.
    free(top->arr);
    free(top);
    scopeStack.pop_back();
}

// procura símbolo nas pilhas de escopo (do topo para a base) e depois nas palavras reservadas
SymbolInfo *findSymbolInScopes(const char *name)
{
    // pesquisar em escopos (do topo pro mais baixo)
    for (int i = (int)scopeStack.size() - 1; i >= 0; --i)
    {
        SymbolInfo *s = searchSymbol(scopeStack[i], name);
        if (s)
            return s;
    }
    // pesquisar tabela global de reservadas (ex.: boolean literals)
    if (globalReserved)
    {
        SymbolInfo *s = searchSymbol(globalReserved, name);
        if (s)
            return s;
    }
    return NULL;
}

// insere símbolo no escopo atual (erro se já existir no mesmo escopo)
bool insertInCurrentScope(SymbolInfo s)
{
    if (scopeStack.empty())
        pushScope();
    hashMap *cur = scopeStack.back();
    // verificação de existência local:
    if (searchSymbol(cur, s.name) != NULL)
        return false;
    insertSymbol(cur, s);
    return true;
}

// --- Erros semânticos ---
void semanticError(int line, const string &msg)
{
    cout << "ERRO SEMÂNTICO na linha " << line << ": " << msg << "\n";
    exit(1);
}

// --- Analisador semântico (passagem sobre tokens) ---
// Implementamos um pequeno parser-semântico que consome tokens e retorna tipos de expressões.
// Usa um índice local `idx` (não altera o vetor original).

// Adaptações: aceita concatenação string + qualquer_coisa -> string.
// Operações aritméticas exigem inteiros.
// &, ^ exigem booleanos.
// Comparações retornam booleano (tipos coerentes: int-int, char-char, bool-bool, string-string).

// Forward declarations
VarType expressionType(const vector<Token> &tokens, int &idx); // consome e devolve tipo
VarType primaryType(const vector<Token> &tokens, int &idx);

// checa existência de token e avança
bool semMatch(const vector<Token> &tokens, int &idx, TokenType t)
{
    if (idx < (int)tokens.size() && tokens[idx].type == t)
    {
        idx++;
        return true;
    }
    return false;
}

void semConsume(const vector<Token> &tokens, int &idx, TokenType t, const string &errMsg)
{
    if (!semMatch(tokens, idx, t))
    {
        int line = (idx < (int)tokens.size()) ? tokens[idx].line : -1;
        semanticError(line, errMsg);
    }
}

// primary: NUM, ID, STRING, BOOLEAN, (expr)
VarType primaryType(const vector<Token> &tokens, int &idx)
{
    if (idx >= (int)tokens.size())
        return TYPE_ERROR;

    Token tk = tokens[idx];

    if (tk.type == TOKEN_NUM)
    {
        idx++;
        return TYPE_INT;
    }
    if (tk.type == TOKEN_STRING)
    {
        idx++;
        return TYPE_STRING;
    }
    if (tk.type == TOKEN_BOOLEAN)
    {
        idx++;
        return TYPE_BOOL;
    }
    if (tk.type == TOKEN_ID)
    {
        // procurar variável nas scopes
        SymbolInfo *s = findSymbolInScopes(tk.lexeme);
        if (!s)
        {
            semanticError(tk.line, string("Variavel '") + tk.lexeme + "' nao declarada");
        }
        idx++;
        // mapear s->type (string) -> VarType
        return typeFromString(s->type);
    }
    if (tk.type == TOKEN_LPAREN)
    {
        idx++;
        VarType t = expressionType(tokens, idx);
        semConsume(tokens, idx, TOKEN_RPAREN, "Esperado ')'");
        return t;
    }

    semanticError(tk.line, string("Expressao primaria invalida (token: ") + tk.lexeme + ")");
    return TYPE_ERROR;
}

// trata operadores de exponenciação (**) -> exige inteiros
VarType exponentType(const vector<Token> &tokens, int &idx)
{
    VarType left = primaryType(tokens, idx);
    while (idx < (int)tokens.size() && tokens[idx].type == TOKEN_EXP)
    {
        Token op = tokens[idx++];
        VarType right = primaryType(tokens, idx);
        if (left != TYPE_INT || right != TYPE_INT)
        {
            semanticError(op.line, "Operador '**' exige operandos do tipo inteiro");
        }
        left = TYPE_INT;
    }
    return left;
}

// *, /, %
VarType termType(const vector<Token> &tokens, int &idx)
{
    VarType left = exponentType(tokens, idx);
    while (idx < (int)tokens.size() &&
           (tokens[idx].type == TOKEN_MUL || tokens[idx].type == TOKEN_DIV || tokens[idx].type == TOKEN_MOD))
    {
        Token op = tokens[idx++];
        VarType right = exponentType(tokens, idx);
        if (left != TYPE_INT || right != TYPE_INT)
        {
            semanticError(op.line, "Operadores '*', '/', '%' exigem operandos inteiros");
        }
        left = TYPE_INT;
    }
    return left;
}

// +, - (note: + pode concatenar strings com qualquer tipo -> resultado string)
VarType arithmeticType(const vector<Token> &tokens, int &idx)
{
    VarType left = termType(tokens, idx);
    while (idx < (int)tokens.size() &&
           (tokens[idx].type == TOKEN_PLUS || tokens[idx].type == TOKEN_MINUS))
    {
        Token op = tokens[idx++];
        VarType right = termType(tokens, idx);
        if (op.type == TOKEN_PLUS)
        {
            // concatenação: se qualquer um for string -> string
            if (left == TYPE_STRING || right == TYPE_STRING)
            {
                left = TYPE_STRING;
            }
            else if (left == TYPE_INT && right == TYPE_INT)
            {
                left = TYPE_INT;
            }
            else
            {
                semanticError(op.line, "Operador '+' entre tipos incompatíveis");
            }
        }
        else
        { // '-'
            if (left != TYPE_INT || right != TYPE_INT)
            {
                semanticError(op.line, "Operador '-' exige operandos inteiros");
            }
            left = TYPE_INT;
        }
    }
    return left;
}

// Comparações: =, <>, >, <, <=, >=  -> retornam booleano
VarType comparisonType(const vector<Token> &tokens, int &idx)
{
    VarType left = arithmeticType(tokens, idx);
    while (idx < (int)tokens.size() &&
           (tokens[idx].type == TOKEN_LT || tokens[idx].type == TOKEN_GT ||
            tokens[idx].type == TOKEN_LE || tokens[idx].type == TOKEN_GE ||
            tokens[idx].type == TOKEN_EQ || tokens[idx].type == TOKEN_NE))
    {
        Token op = tokens[idx++];
        VarType right = arithmeticType(tokens, idx);
        // tipos compatíveis: int-int, char-char, bool-bool, string-string
        if (left != right)
        {
            semanticError(op.line, string("Comparacao entre tipos incompativeis: ") + typeName(left) + " e " + typeName(right));
        }
        left = TYPE_BOOL;
    }
    return left;
}

// AND & OR ^ -> exigem booleanos
VarType logicType(const vector<Token> &tokens, int &idx)
{
    VarType left = comparisonType(tokens, idx);
    while (idx < (int)tokens.size() && (tokens[idx].type == TOKEN_AND || tokens[idx].type == TOKEN_OR))
    {
        Token op = tokens[idx++];
        VarType right = comparisonType(tokens, idx);
        if (left != TYPE_BOOL || right != TYPE_BOOL)
        {
            semanticError(op.line, "Operadores logicos '&' e '^' exigem operandos booleanos");
        }
        left = TYPE_BOOL;
    }
    return left;
}

VarType expressionType(const vector<Token> &tokens, int &idx)
{
    return logicType(tokens, idx);
}

// --- Verifica uma declaração: tipo lista_ids ;  (ex.: inteiro a <- 1, b;)
// idx aponta para o token do tipo (TIPO_INTEIRO etc.)
void declarationListSemantic(const vector<Token> &tokens, int &idx)
{
    Token tipoToken = tokens[idx++];
    const char *typeStr = nullptr;
    if (tipoToken.type == TOKEN_TIPO_INTEIRO)
        typeStr = "inteiro";
    else if (tipoToken.type == TOKEN_TIPO_LOGICO)
        typeStr = "logico";
    else if (tipoToken.type == TOKEN_TIPO_CARACTERE)
        typeStr = "caractere";
    else
        semanticError(tipoToken.line, "Tipo desconhecido em declaracao");

    // primeiro identificador
    semConsume(tokens, idx, TOKEN_ID, "Esperado identificador apos tipo");
    Token idtk = tokens[idx - 1];

    // criar símbolo e inserir no escopo atual
    SymbolInfo s = createSymbol(idtk.lexeme, typeStr, TOKEN_ID, idtk.line);
    // createSymbol já define sizeInBytes e memory para tipos conhecidos
    if (!insertInCurrentScope(s))
    {
        semanticError(idtk.line, string("Variavel '") + idtk.lexeme + "' ja declarada no escopo atual");
    }

    // possivel atribuição
    if (semMatch(tokens, idx, TOKEN_ASSIGN))
    {
        int exprIdx = idx;
        VarType t = expressionType(tokens, exprIdx);
        // verificar compatibilidade
        VarType varType = typeFromString(s.type);
        // se char e string, string length must be 1 -> we can check token if was literal
        if (t == TYPE_STRING && varType == TYPE_CHAR)
        {
            // check that expression was a string literal with length 1
            // we only can check if the expression was a single TOKEN_STRING literal:
            if (!(tokens[idx].type == TOKEN_STRING))
            {
                semanticError(idtk.line, "Atribuicao invalida: esperado string literal de 1 char para 'caractere'");
            }
            else
            {
                // length check (lexeme already without quotes)
                if (strlen(tokens[idx].lexeme) != 1)
                {
                    semanticError(tokens[idx].line, "Atribuicao invalida: string com mais de 1 caractere para tipo 'caractere'");
                }
            }
        }
        else if (t != varType && !(t == TYPE_INT && varType == TYPE_CHAR))
        {
            // allow int->char? usually not. We'll be strict.
            semanticError(idtk.line, string("Atribuicao invalida: tipo da expressao (") + typeName(t) + ") diferente de " + typeName(varType));
        }
        // advance idx to exprIdx
        idx = exprIdx;
    }

    // possiveis outras variaveis separadas por ,
    while (semMatch(tokens, idx, TOKEN_COMMA))
    {
        semConsume(tokens, idx, TOKEN_ID, "Esperado identificador apos ','");
        Token idtk2 = tokens[idx - 1];
        SymbolInfo s2 = createSymbol(idtk2.lexeme, typeStr, TOKEN_ID, idtk2.line);
        if (!insertInCurrentScope(s2))
        {
            semanticError(idtk2.line, string("Variavel '") + idtk2.lexeme + "' ja declarada no escopo atual");
        }
        if (semMatch(tokens, idx, TOKEN_ASSIGN))
        {
            int exprIdx = idx;
            VarType t = expressionType(tokens, exprIdx);
            VarType varType = typeFromString(s2.type);
            if (t == TYPE_STRING && varType == TYPE_CHAR)
            {
                if (!(tokens[idx].type == TOKEN_STRING))
                    semanticError(idtk2.line, "Atribuicao invalida: esperado string literal de 1 char para 'caractere'");
                if (strlen(tokens[idx].lexeme) != 1)
                    semanticError(tokens[idx].line, "Atribuicao invalida: string com mais de 1 caractere para tipo 'caractere'");
            }
            else if (t != varType)
            {
                semanticError(idtk2.line, string("Atribuicao invalida: tipo da expressao (") + typeName(t) + ") diferente de " + typeName(varType));
            }
            idx = exprIdx;
        }
    }

    semConsume(tokens, idx, TOKEN_SEMICOLON, "Esperado ';' apos declaracao");
}

// assignment semântico: ID <- expr ;
void assignmentSemantic(const vector<Token> &tokens, int &idx)
{
    Token idtk = tokens[idx++];
    if (idtk.type != TOKEN_ID)
        semanticError(idtk.line, "Esperado identificador em atribuicao");
    SymbolInfo *var = findSymbolInScopes(idtk.lexeme);
    if (!var)
        semanticError(idtk.line, string("Variavel '") + idtk.lexeme + "' nao declarada");

    semConsume(tokens, idx, TOKEN_ASSIGN, "Esperado '<-' em atribuicao");
    int exprIdx = idx;
    VarType t = expressionType(tokens, exprIdx);

    VarType varType = typeFromString(var->type);
    if (t == TYPE_STRING && varType == TYPE_CHAR)
    {
        // only allowed if string literal of length 1
        if (tokens[idx].type != TOKEN_STRING || strlen(tokens[idx].lexeme) != 1)
        {
            semanticError(idtk.line, "Atribuicao invalida: string com mais de 1 caractere para tipo 'caractere'");
        }
    }
    else if (t != varType)
    {
        // allow concatenation->string only in expressions, assignment must match var type
        semanticError(idtk.line, string("Atribuicao: tipo da expressao (") + typeName(t) + ") diferente de " + typeName(varType));
    }

    idx = exprIdx;
    semConsume(tokens, idx, TOKEN_SEMICOLON, "Esperado ';' apos atribuicao");
}

// imprimir: Imprimir ( expressão ) ;
void printSemantic(const vector<Token> &tokens, int &idx)
{
    Token tk = tokens[idx - 1]; // 'imprimir' já consumido
    semConsume(tokens, idx, TOKEN_LPAREN, "Esperado '(' apos imprimir");
    int exprIdx = idx;
    VarType t = expressionType(tokens, exprIdx);
    // imprimir aceita qualquer tipo (string,int,char,bool) — ok
    idx = exprIdx;
    semConsume(tokens, idx, TOKEN_RPAREN, "Esperado ')' apos imprimir");
    semConsume(tokens, idx, TOKEN_SEMICOLON, "Esperado ';' apos imprimir");
}

// if / else
void ifSemantic(const vector<Token> &tokens, int &idx)
{
    semConsume(tokens, idx, TOKEN_LPAREN, "Esperado '(' apos 'se'");
    int exprIdx = idx;
    VarType t = expressionType(tokens, exprIdx);
    if (t != TYPE_BOOL)
        semanticError(tokens[idx].line, "Condicao de 'se' deve ser do tipo logico");
    idx = exprIdx;
    semConsume(tokens, idx, TOKEN_RPAREN, "Esperado ')'");
    // bloco: push scope, analisar, pop scope
    pushScope();
    semConsume(tokens, idx, TOKEN_LBRACE, "Esperado '{' apos condicao");
    // analisar statements dentro do bloco até '}'
    while (!semMatch(tokens, idx, TOKEN_RBRACE))
    {
        // use main dispatcher (see below)
        // avoid recursion by calling a dispatcher function:
        // We'll call analyzeStatement()
        // forward declare in outer scope; here we call it (it must be defined below).
        // placeholder
        break;
    }
    popScope();
    // check if next token is 'senao'
    if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_ELSE)
    {
        idx++;
        pushScope();
        semConsume(tokens, idx, TOKEN_LBRACE, "Esperado '{' apos 'senao'");
        while (!semMatch(tokens, idx, TOKEN_RBRACE))
        {
            break;
        }
        popScope();
    }
}

// while semantic
void whileSemantic(const vector<Token> &tokens, int &idx)
{
    semConsume(tokens, idx, TOKEN_LPAREN, "Esperado '(' apos 'enquanto'");
    int exprIdx = idx;
    VarType t = expressionType(tokens, exprIdx);
    if (t != TYPE_BOOL)
        semanticError(tokens[idx].line, "Condicao de 'enquanto' deve ser logica");
    idx = exprIdx;
    semConsume(tokens, idx, TOKEN_RPAREN, "Esperado ')'");
    pushScope();
    semConsume(tokens, idx, TOKEN_LBRACE, "Esperado '{' apos 'enquanto'");
    while (!semMatch(tokens, idx, TOKEN_RBRACE))
    {
        break;
    }
    popScope();
}

// for semantic: Para i em (inicio, fim, passo) { ... }
// semantics: 'i' is declared as inteiro in loop scope only
void forSemantic(const vector<Token> &tokens, int &idx)
{
    semConsume(tokens, idx, TOKEN_ID, "Esperado identificador do iterador apos 'para'");
    Token idtk = tokens[idx - 1];
    semConsume(tokens, idx, TOKEN_EM, "Esperado 'em' apos identificador do for");
    semConsume(tokens, idx, TOKEN_LPAREN, "Esperado '(' apos 'em'");

    // inicio
    int exprIdx = idx;
    VarType t1 = expressionType(tokens, exprIdx);
    if (t1 != TYPE_INT)
        semanticError(tokens[idx].line, "Inicio do 'para' deve ser inteiro");
    idx = exprIdx;
    semConsume(tokens, idx, TOKEN_COMMA, "Esperado ',' apos inicio do intervalo");

    // fim
    exprIdx = idx;
    VarType t2 = expressionType(tokens, exprIdx);
    if (t2 != TYPE_INT)
        semanticError(tokens[idx].line, "Fim do 'para' deve ser inteiro");
    idx = exprIdx;
    semConsume(tokens, idx, TOKEN_COMMA, "Esperado ',' apos fim do intervalo");

    // passo
    exprIdx = idx;
    VarType t3 = expressionType(tokens, exprIdx);
    if (t3 != TYPE_INT)
        semanticError(tokens[idx].line, "Passo do 'para' deve ser inteiro");
    idx = exprIdx;
    semConsume(tokens, idx, TOKEN_RPAREN, "Esperado ')' apos parametros do for");

    // create loop variable in new scope
    pushScope();
    SymbolInfo s = createSymbol(idtk.lexeme, "inteiro", TOKEN_ID, idtk.line);
    if (!insertInCurrentScope(s))
        semanticError(idtk.line, string("Iterador '") + idtk.lexeme + "' ja declarado no escopo atual do for");

    // bloco
    semConsume(tokens, idx, TOKEN_LBRACE, "Esperado '{' apos for");
    while (!semMatch(tokens, idx, TOKEN_RBRACE))
    {
        break;
    }
    popScope();
}

// Dispatcher forward declaration
void analyzeStatement(const vector<Token> &tokens, int &idx);

// Now implement the dispatcher
void analyzeStatement(const vector<Token> &tokens, int &idx)
{
    if (idx >= (int)tokens.size())
        return;
    Token tk = tokens[idx];

    // types declarations
    if (tk.type == TOKEN_TIPO_INTEIRO || tk.type == TOKEN_TIPO_LOGICO || tk.type == TOKEN_TIPO_CARACTERE)
    {
        declarationListSemantic(tokens, idx);
        return;
    }

    if (tk.type == TOKEN_ID)
    {
        // assignment
        assignmentSemantic(tokens, idx);
        return;
    }

    if (tk.type == TOKEN_PRINT)
    {
        idx++; // consume imprimir
        printSemantic(tokens, idx);
        return;
    }

    if (tk.type == TOKEN_IF)
    {
        idx++;
        // for proper block handling we'll implement simpler: reuse parser-like handling
        semConsume(tokens, idx, TOKEN_LPAREN, "Esperado '(' apos 'se'");
        int exprIdx = idx;
        VarType t = expressionType(tokens, exprIdx);
        if (t != TYPE_BOOL)
            semanticError(tokens[idx].line, "Condicao de 'se' deve ser logica");
        idx = exprIdx;
        semConsume(tokens, idx, TOKEN_RPAREN, "Esperado ')'");
        // bloco
        semConsume(tokens, idx, TOKEN_LBRACE, "Esperado '{' apos 'se'");
        pushScope();
        while (!semMatch(tokens, idx, TOKEN_RBRACE))
        {
            analyzeStatement(tokens, idx);
        }
        popScope();
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_ELSE)
        {
            idx++; // consume else
            semConsume(tokens, idx, TOKEN_LBRACE, "Esperado '{' apos 'senao'");
            pushScope();
            while (!semMatch(tokens, idx, TOKEN_RBRACE))
            {
                analyzeStatement(tokens, idx);
            }
            popScope();
        }
        return;
    }

    if (tk.type == TOKEN_WHILE)
    {
        idx++;
        semConsume(tokens, idx, TOKEN_LPAREN, "Esperado '(' apos 'enquanto'");
        int exprIdx = idx;
        VarType t = expressionType(tokens, exprIdx);
        if (t != TYPE_BOOL)
            semanticError(tokens[idx].line, "Condicao de 'enquanto' deve ser logica");
        idx = exprIdx;
        semConsume(tokens, idx, TOKEN_RPAREN, "Esperado ')'");
        semConsume(tokens, idx, TOKEN_LBRACE, "Esperado '{' apos 'enquanto'");
        pushScope();
        while (!semMatch(tokens, idx, TOKEN_RBRACE))
        {
            analyzeStatement(tokens, idx);
        }
        popScope();
        return;
    }

    if (tk.type == TOKEN_FOR)
    {
        idx++;
        semConsume(tokens, idx, TOKEN_ID, "Esperado identificador do iterador apos 'para'");
        Token idtk = tokens[idx - 1];
        semConsume(tokens, idx, TOKEN_EM, "Esperado 'em' apos identificador do for");
        semConsume(tokens, idx, TOKEN_LPAREN, "Esperado '(' apos 'em'");
        // inicio
        int exprIdx = idx;
        VarType t1 = expressionType(tokens, exprIdx);
        if (t1 != TYPE_INT)
            semanticError(tokens[idx].line, "Inicio do 'para' deve ser inteiro");
        idx = exprIdx;
        semConsume(tokens, idx, TOKEN_COMMA, "Esperado ',' apos inicio do intervalo");
        // fim
        exprIdx = idx;
        VarType t2 = expressionType(tokens, exprIdx);
        if (t2 != TYPE_INT)
            semanticError(tokens[idx].line, "Fim do 'para' deve ser inteiro");
        idx = exprIdx;
        semConsume(tokens, idx, TOKEN_COMMA, "Esperado ',' apos fim do intervalo");
        // passo
        exprIdx = idx;
        VarType t3 = expressionType(tokens, exprIdx);
        if (t3 != TYPE_INT)
            semanticError(tokens[idx].line, "Passo do 'para' deve ser inteiro");
        idx = exprIdx;
        semConsume(tokens, idx, TOKEN_RPAREN, "Esperado ')' apos parametros do for");
        // create iter variable in loop scope
        pushScope();
        SymbolInfo s = createSymbol(idtk.lexeme, "inteiro", TOKEN_ID, idtk.line);
        if (!insertInCurrentScope(s))
            semanticError(idtk.line, string("Iterador '") + idtk.lexeme + "' ja declarado no escopo do for");
        // bloco
        semConsume(tokens, idx, TOKEN_LBRACE, "Esperado '{' apos for");
        while (!semMatch(tokens, idx, TOKEN_RBRACE))
        {
            analyzeStatement(tokens, idx);
        }
        popScope();
        return;
    }

    // If token is just a '}' or EOF, return (caller handles)
    if (tk.type == TOKEN_RBRACE || tk.type == TOKEN_EOF)
    {
        return;
    }

    // any other token -> error
    semanticError(tk.line, string("Comando semantico invalido (token: ") + tk.lexeme + ")");
}

// --- Função pública: realiza a análise semântica sobre os tokens gerados ---
void semanticAnalyzer(const vector<Token> &ALL_TOKENS, hashMap *reservedTable)
{
    globalReserved = reservedTable;
    // inicia com escopo global vazio (para variáveis do programa)
    pushScope();

    int idx = 0;
    // consumir até EOF
    while (idx < (int)ALL_TOKENS.size() && ALL_TOKENS[idx].type != TOKEN_EOF)
    {
        analyzeStatement(ALL_TOKENS, idx);
    }

    // finaliza, limpa escopos
    while (!scopeStack.empty())
        popScope();
}
