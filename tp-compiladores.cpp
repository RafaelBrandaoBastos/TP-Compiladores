// C++ Program to Read a file line by line using getline
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include "hash-table.h"
#include <cstring>  

using namespace std;

void lexicalAnalyzer(string line, struct hashMap* SymbolTable) {
    string token;
    char* match;
    for (char c : line) {
        token += c;
        match = search(SymbolTable, (char*)token.c_str());
        cout << "Current-Token: " << token << endl;
        cout << match << endl;
        std::string mtch = token;
        if(match != "404") {
            if (match = "for"){

            } else if(match = "int") {

            } else if(match = "bool") {
                
            }
        }
    }
}

int main()
{
    //Abre arquivo de texto
    ifstream file("code.txt");
    //Cria arquivo assembly
    ofstream MyFile("result.asm");
    // String para armazenar linhas de código
    string line;
    
    //Inicializando tabela de símbolos
    struct hashMap* SymbolTable = (struct hashMap*)malloc(sizeof(struct hashMap));
    initializeHashMap(SymbolTable);


    //Lógica Leitura de arquivo
    if (file.is_open())
    {
        insert(SymbolTable, "elite_Programmer", "Manish");
        // Read each line from the file and store it in the 'line' variable.
        while (getline(file, line))
        {
			lexicalAnalyzer(line, SymbolTable);
        }
        // Close the file stream once all lines have been read.
        file.close();
    }
    else
    {
        // Print an error message to the standard error stream if the file cannot be opened.
        cerr << "Unable to open file!" << endl;
    }

    return 0;
}