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


/*
ANALISADOR LEXICO:

-Lê os caracteres
-Agrupa caracteres formando tokens
-Filtra comentários
-Reconhece números, identificadores, operadores, strings
-Consulta tabela de símbolos somente para saber se é palavra reservada
-Só produz tokens, não interpreta
*/



/*
void lexicalAnalyzer(string line, struct hashMap* SymbolTable) {
    string token;
    char* match;
    for (char c : line) {
        token += c;
        match = searchSymbol(SymbolTable, (char*)token.c_str());
        cout << "Current-Token: " << token << endl;
        cout << match << endl;
        std::string mtch = token;
        if(strcmp(match, "404") != 0) {
            if(strcmp(match, "for") == 0) {

            } else if(match = "int") {

            } else if(match = "bool") {
                
            }
        }
    }
}

*/


vector<Token> lexicalAnalyzer(string line, int lineNumber, hashMap* table) {

    vector<Token> tokens;
    int i = 0;
    int n = line.size();

    while (i < n) {

        // Ignorar espaços
        if (isspace(line[i])) {
            i++;
            continue;
        }

        // Comentário de 1 linha: $
        if (line[i] == '$' && (i+1 >= n || line[i+1] != '$')) {
            break;
        }

        // Comentário de múltiplas linhas $$ ... $$
        if (line[i] == '$' && i+1 < n && line[i+1] == '$') {
            i += 2;
            while (i+1 < n && !(line[i] == '$' && line[i+1] == '$'))
                i++;

            if (i+1 < n) i += 2;
            continue;
        }

        // ---------------- STRINGS ----------------
        if (line[i] == '"') {
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
        if (isalpha(line[i])) {

            string tok = "";
            while (i < n && isalpha(line[i])) {
                tok += tolower(line[i]);
                i++;
            }

            SymbolInfo* sym = searchSymbol(table, tok.c_str());

            if (sym) {
                // palavra reservada
                Token tk = { (TokenType)sym->tokenType, "", lineNumber };
                strcpy(tk.lexeme, tok.c_str());
                tokens.push_back(tk);
            } 
            else {
                // identificador novo → inserir tabela
                SymbolInfo s = createSymbol(tok.c_str(), "id", TOKEN_ID, lineNumber);
                insertSymbol(table, s);

                Token tk = { TOKEN_ID, "", lineNumber };
                strcpy(tk.lexeme, tok.c_str());
                tokens.push_back(tk);
            }
            continue;
        }

        // ---------------- NÚMEROS ----------------
        if (isdigit(line[i])) {

            string num = "";
            while (i < n && isdigit(line[i]))
                num += line[i++];

            Token tk = {TOKEN_NUM, "", lineNumber};
            strcpy(tk.lexeme, num.c_str());
            tokens.push_back(tk);
            continue;
        }

        // ---------------- OPERADORES DUPLOS ----------------
        if (i+1 < n) {
            string op2 = line.substr(i, 2);

            SymbolInfo* sym = searchSymbol(table, op2.c_str());
            if (sym) {
                Token tk = { (TokenType)sym->tokenType, "", lineNumber };
                strcpy(tk.lexeme, op2.c_str());
                tokens.push_back(tk);
                i += 2;
                continue;
            }
        }

        // ---------------- OPERADORES SIMPLES ----------------
        string op1 = line.substr(i, 1);
        SymbolInfo* sym2 = searchSymbol(table, op1.c_str());

        if (sym2) {
            Token tk = { (TokenType)sym2->tokenType, "", lineNumber };
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


/*
ANALISADOR SINTÁTICO (PARSER):

- Recebe a lista de tokens gerados pelo léxico
- Verifica se a sequência de tokens segue as regras da gramática
- Detecta erros de sintaxe (token inesperado, falta de ';', etc.)
- Organiza os tokens 
- Garante que a estrutura do programa é válida
*/


void Parser (){

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
    if (!input.is_open()) {
        cout << "Erro: nao foi possivel abrir input.txt\n";
        return 1;
    }
    else{
        cout << "Arquivo input.txt aberto com sucesso\n";}

    // 4. Cria o arquivo de saída dos tokens
    ofstream output("tokens.txt");
    if (!output.is_open()) {
        cout << "Erro: nao foi possivel criar tokens.txt\n";
        return 1;
    }

    
    // 5. Ler linha por linha e transformar em tokens
    string line;
    int lineNumber = 1;

    while (getline(input, line)) {

        vector<Token> tokens = lexicalAnalyzer(line, lineNumber, &SymbolTable);

        for (const Token& t : tokens) {
            output << tokenTypeName(t.type)
                   << "  \"" << t.lexeme << "\""
                   << "   linha " << t.line << "\n";
        }
        lineNumber++;
    }

    // 6. Token EOF
    output << "EOF\n";

    
    // 7. Fecha arquivos
    input.close();
    output.close();

    cout << "Análise léxica concluída. Tokens salvos em tokens.txt.\n";


    return 0;
}
