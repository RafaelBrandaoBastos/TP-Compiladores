// codegen.h
#ifndef CODEGEN_H
#define CODEGEN_H

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "tokens.h"
#include "hash-table.h"

using namespace std;

// Simple MIPS code generator (RARS/MARS friendly)
// - Assumes lexical/syntax/semantic phases already ran.
// - Expects Token.lexeme to contain identifiers/literals (as in your tokens.h).
// - Uses $t9 as base pointer for a tmp-space in .data: 0($t9), 4($t9), ...

struct CodeGenerator
{
    hashMap *reservedTable; // pointer to global symbol/reserved table (passed by caller)
    ofstream out;
    int labelCounter = 0;
    int tempCounter = 0;
    unordered_map<string, string> varLabel; // var name -> label in .data
    unordered_map<string, string> varType;  // var name -> type string (if available from symbol table)
    vector<string> data_strings;

    CodeGenerator(hashMap *reserved = nullptr)
    {
        reservedTable = reserved;
    }

    string newLabel(const string &base)
    {
        return base + "_" + to_string(labelCounter++);
    }

    string ensureVarLabel(const char *name)
    {
        string s(name);
        auto it = varLabel.find(s);
        if (it != varLabel.end())
            return it->second;
        string lbl = "var_" + s;
        varLabel[s] = lbl;
        // try to fetch type from reservedTable / global symbol table if present
        if (reservedTable)
        {
            SymbolInfo *si = searchSymbol(reservedTable, s.c_str());
            if (si)
                varType[s] = string(si->type);
        }
        return lbl;
    }

    string addStringLiteral(const string &s)
    {
        for (size_t i = 0; i < data_strings.size(); ++i)
            if (data_strings[i] == s)
                return "str_" + to_string(i);
        data_strings.push_back(s);
        return "str_" + to_string((int)data_strings.size() - 1);
    }

    string escapeString(const string &s)
    {
        string out;
        for (char c : s)
        {
            if (c == '"')
                out += "\\\"";
            else if (c == '\\')
                out += "\\\\";
            else if (c == '\n')
                out += "\\n";
            else
                out += c;
        }
        return out;
    }

    // Scan tokens to reserve variable labels and collect string literals
    void scanAndReserveVars(const vector<Token> &tokens)
    {
        int i = 0;
        while (i < (int)tokens.size())
        {
            Token tk = tokens[i];
            if (tk.type == TOKEN_TIPO_INTEIRO || tk.type == TOKEN_TIPO_LOGICO || tk.type == TOKEN_TIPO_CARACTERE)
            {
                // declaration sequence: TYPE ID (maybe <- expr) (, ID ... ) ;
                int typeTok = tk.type;
                i++;
                // one or more ids
                while (i < (int)tokens.size() && tokens[i].type != TOKEN_SEMICOLON)
                {
                    if (tokens[i].type == TOKEN_ID)
                    {
                        ensureVarLabel(tokens[i].lexeme);
                        // optionally read assign and collect strings in expr
                        int j = i + 1;
                        if (j < (int)tokens.size() && tokens[j].type == TOKEN_ASSIGN)
                        {
                            j++;
                            // scan until comma or semicolon
                            while (j < (int)tokens.size() && tokens[j].type != TOKEN_COMMA && tokens[j].type != TOKEN_SEMICOLON)
                            {
                                if (tokens[j].type == TOKEN_STRING)
                                    addStringLiteral(tokens[j].lexeme);
                                j++;
                            }
                        }
                        i++;
                    }
                    else
                    {
                        if (tokens[i].type == TOKEN_STRING)
                            addStringLiteral(tokens[i].lexeme);
                        i++;
                    }
                    if (i < (int)tokens.size() && tokens[i].type == TOKEN_COMMA)
                        i++;
                }
                if (i < (int)tokens.size() && tokens[i].type == TOKEN_SEMICOLON)
                    i++;
                continue;
            }

            if (tk.type == TOKEN_PRINT)
            {
                // capture strings until semicolon
                i++;
                while (i < (int)tokens.size() && tokens[i].type != TOKEN_SEMICOLON)
                {
                    if (tokens[i].type == TOKEN_STRING)
                        addStringLiteral(tokens[i].lexeme);
                    i++;
                }
                if (i < (int)tokens.size() && tokens[i].type == TOKEN_SEMICOLON)
                    i++;
                continue;
            }

            // general scan for string literals in expressions
            if (tk.type == TOKEN_STRING)
                addStringLiteral(tk.lexeme);
            i++;
        }
    }

    // Top-level generate
    void generate(const vector<Token> &ALL_TOKENS, const string &filename)
    {
        out.open(filename);
        if (!out.is_open())
        {
            fprintf(stderr, "Erro: nao foi possivel criar %s\n", filename.c_str());
            return;
        }

        out << "# MIPS gerado automaticamente (RARS/MARS friendly)\n";
        out << ".data\n";
        out << "tmp_space: .space 1024\n";

        scanAndReserveVars(ALL_TOKENS);

        for (size_t i = 0; i < data_strings.size(); ++i)
        {
            out << "str_" << i << ": .asciiz \"" << escapeString(data_strings[i]) << "\"\n";
        }

        // Emit variables (as words). We keep .word for simplicity; can refine later.
        for (auto &p : varLabel)
        {
            out << p.second << ": .word 0\n";
        }

        out << ".text\n.globl main\nmain:\n";
        // setup base of tmp_space
        out << "la $t9, tmp_space\n";

        int idx = 0;
        while (idx < (int)ALL_TOKENS.size() && ALL_TOKENS[idx].type != TOKEN_EOF)
        {
            generateStatement(ALL_TOKENS, idx);
        }

        // exit
        out << "li $v0, 10\nsyscall\n";

        out.close();
        printf("Assembly gerado: %s\n", filename.c_str());
    }

    // -------------------- Statement / block / flow --------------------

    void generateStatement(const vector<Token> &tokens, int &idx)
    {
        if (idx >= (int)tokens.size())
            return;
        Token tk = tokens[idx];

        // Declaration: TYPE id ( <- expr ) (, id ... ) ;
        if (tk.type == TOKEN_TIPO_INTEIRO || tk.type == TOKEN_TIPO_LOGICO || tk.type == TOKEN_TIPO_CARACTERE)
        {
            idx++; // consume type
            // first id or more
            while (idx < (int)tokens.size() && tokens[idx].type != TOKEN_SEMICOLON)
            {
                if (tokens[idx].type == TOKEN_ID)
                {
                    string name = tokens[idx].lexeme;
                    ensureVarLabel(name.c_str());
                    idx++;
                    // optional assignment
                    if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_ASSIGN)
                    {
                        idx++;                           // consume '<-'
                        generateExpression(tokens, idx); // result in $t0
                        string lbl = ensureVarLabel(name.c_str());
                        out << "sw $t0, " << lbl << "\n";
                    }
                }
                else
                {
                    idx++;
                }
                if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_COMMA)
                    idx++;
            }
            if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_SEMICOLON)
                idx++;
            return;
        }

        // Assignment: ID <- expr ;
        if (tk.type == TOKEN_ID)
        {
            string name = tk.lexeme;
            idx++;
            if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_ASSIGN)
            {
                idx++;                           // consume '<-'
                generateExpression(tokens, idx); // result in $t0
                string lbl = ensureVarLabel(name.c_str());
                out << "sw $t0, " << lbl << "\n";
            }
            else
            {
                // not an assignment; skip to semicolon to be safe
                while (idx < (int)tokens.size() && tokens[idx].type != TOKEN_SEMICOLON)
                    idx++;
            }
            if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_SEMICOLON)
                idx++;
            return;
        }

        // Print
        if (tk.type == TOKEN_PRINT)
        {
            idx++; // consume 'imprimir'
            generatePrint(tokens, idx);
            if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_SEMICOLON)
                idx++;
            return;
        }

        // If / Else
        if (tk.type == TOKEN_IF)
        {
            generateIf(tokens, idx);
            return;
        }

        // While
        if (tk.type == TOKEN_WHILE)
        {
            generateWhile(tokens, idx);
            return;
        }

        // For
        if (tk.type == TOKEN_FOR)
        {
            generateFor(tokens, idx);
            return;
        }

        // Skip braces or semicolons
        if (tk.type == TOKEN_RBRACE || tk.type == TOKEN_SEMICOLON)
        {
            idx++;
            return;
        }

        // fallback: skip unknown token
        idx++;
    }

    void generateBlock(const vector<Token> &tokens, int &idx)
    {
        if (idx >= (int)tokens.size())
            return;
        if (tokens[idx].type != TOKEN_LBRACE)
        {
            // single statement (not a block)
            generateStatement(tokens, idx);
            return;
        }
        idx++; // consume '{'
        while (idx < (int)tokens.size() && tokens[idx].type != TOKEN_RBRACE)
        {
            generateStatement(tokens, idx);
        }
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_RBRACE)
            idx++; // consume '}'
    }

    void generateIf(const vector<Token> &tokens, int &idx)
    {
        idx++; // consume 'se'
        if (idx >= (int)tokens.size())
            return;
        if (tokens[idx].type == TOKEN_LPAREN)
            idx++;
        generateExpression(tokens, idx); // cond -> $t0
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_RPAREN)
            idx++;

        string elseLbl = newLabel("else");
        string endLbl = newLabel("endif");

        out << "beq $t0, $zero, " << elseLbl << "\n";
        // then
        generateBlock(tokens, idx);
        out << "j " << endLbl << "\n";
        // else
        out << elseLbl << ":\n";
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_ELSE)
        {
            idx++; // consume 'senao'
            generateBlock(tokens, idx);
        }
        out << endLbl << ":\n";
    }

    void generateWhile(const vector<Token> &tokens, int &idx)
    {
        idx++; // consume 'enquanto'
        string start = newLabel("while_start");
        string end = newLabel("while_end");
        out << start << ":\n";
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_LPAREN)
            idx++;
        generateExpression(tokens, idx); // cond -> $t0
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_RPAREN)
            idx++;
        out << "beq $t0, $zero, " << end << "\n";
        generateBlock(tokens, idx);
        out << "j " << start << "\n";
        out << end << ":\n";
    }

    void generateFor(const vector<Token> &tokens, int &idx)
    {
        idx++; // consume 'para'
        if (idx >= (int)tokens.size())
            return;
        string iterName = "i";
        if (tokens[idx].type == TOKEN_ID)
        {
            iterName = tokens[idx].lexeme;
            idx++;
        }
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_EM)
            idx++;
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_LPAREN)
            idx++;

        // start
        generateExpression(tokens, idx); // start -> $t0
        string iterLbl = ensureVarLabel(iterName.c_str());
        out << "sw $t0, " << iterLbl << "\n";

        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_COMMA)
            idx++;

        // end
        generateExpression(tokens, idx); // end -> $t0
        out << "sw $t0, 0($t9)\n";       // store end at 0($t9)

        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_COMMA)
            idx++;

        // step
        generateExpression(tokens, idx); // step -> $t0
        out << "sw $t0, 4($t9)\n";       // store step at 4($t9)

        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_RPAREN)
            idx++;

        string startLbl = newLabel("for_start");
        string endLbl = newLabel("for_end");
        out << startLbl << ":\n";
        out << "lw $t0, " << iterLbl << "\n";
        out << "lw $t1, 0($t9)\n";                 // end
        out << "bgt $t0, $t1, " << endLbl << "\n"; // assumes positive step
        generateBlock(tokens, idx);
        // increment
        out << "lw $t0, " << iterLbl << "\n";
        out << "lw $t1, 4($t9)\n";
        out << "addu $t0, $t0, $t1\n";
        out << "sw $t0, " << iterLbl << "\n";
        out << "j " << startLbl << "\n";
        out << endLbl << ":\n";
    }

    // -------------------- Expressions (result in $t0) --------------------

    string generateExpression(const vector<Token> &tokens, int &idx)
    {
        return genLogic(tokens, idx);
    }

    // logic: comparisons with & ^ (AND, OR)
    string genLogic(const vector<Token> &tokens, int &idx)
    {
        genComparison(tokens, idx); // result -> $t0
        while (idx < (int)tokens.size() && (tokens[idx].type == TOKEN_AND || tokens[idx].type == TOKEN_OR))
        {
            Token op = tokens[idx++]; // consume op
            out << "sw $t0, 8($t9)\n";
            genComparison(tokens, idx); // right -> $t0
            out << "lw $t1, 8($t9)\n";  // left -> $t1
            if (op.type == TOKEN_AND)
                out << "and $t0, $t1, $t0\n";
            else
                out << "or $t0, $t1, $t0\n";
        }
        return "$t0";
    }

    // comparisons: <, >, <=, >=, =, <>
    string genComparison(const vector<Token> &tokens, int &idx)
    {
        genArithmetic(tokens, idx); // left -> $t0
        while (idx < (int)tokens.size() &&
               (tokens[idx].type == TOKEN_LT || tokens[idx].type == TOKEN_GT ||
                tokens[idx].type == TOKEN_LE || tokens[idx].type == TOKEN_GE ||
                tokens[idx].type == TOKEN_EQ || tokens[idx].type == TOKEN_NE))
        {
            Token op = tokens[idx++];
            out << "sw $t0, 12($t9)\n";
            genArithmetic(tokens, idx); // right -> $t0
            out << "lw $t1, 12($t9)\n"; // left -> $t1
            if (op.type == TOKEN_LT)
                out << "slt $t2, $t1, $t0\n";
            else if (op.type == TOKEN_GT)
                out << "slt $t2, $t0, $t1\n";
            else if (op.type == TOKEN_EQ)
                out << "seq $t2, $t1, $t0\n";
            else if (op.type == TOKEN_NE)
                out << "sne $t2, $t1, $t0\n";
            else if (op.type == TOKEN_LE)
            {
                out << "slt $t2, $t0, $t1\n";
                out << "xori $t2, $t2, 1\n";
            }
            else if (op.type == TOKEN_GE)
            {
                out << "slt $t2, $t1, $t0\n";
                out << "xori $t2, $t2, 1\n";
            }
            out << "move $t0, $t2\n";
        }
        return "$t0";
    }

    // arithmetic: +, -
    string genArithmetic(const vector<Token> &tokens, int &idx)
    {
        genTerm(tokens, idx); // left -> $t0
        while (idx < (int)tokens.size() && (tokens[idx].type == TOKEN_PLUS || tokens[idx].type == TOKEN_MINUS))
        {
            Token op = tokens[idx++];
            out << "sw $t0, 16($t9)\n";
            genTerm(tokens, idx); // right -> $t0
            out << "lw $t1, 16($t9)\n";
            if (op.type == TOKEN_PLUS)
                out << "addu $t0, $t1, $t0\n";
            else
                out << "subu $t0, $t1, $t0\n";
        }
        return "$t0";
    }

    // term: *, /, %
    string genTerm(const vector<Token> &tokens, int &idx)
    {
        genExponent(tokens, idx);
        while (idx < (int)tokens.size() && (tokens[idx].type == TOKEN_MUL || tokens[idx].type == TOKEN_DIV || tokens[idx].type == TOKEN_MOD))
        {
            Token op = tokens[idx++];
            out << "sw $t0, 20($t9)\n";
            genExponent(tokens, idx);
            out << "lw $t1, 20($t9)\n";
            if (op.type == TOKEN_MUL)
                out << "mul $t0, $t1, $t0\n";
            else if (op.type == TOKEN_DIV)
            {
                out << "div $t1, $t0\n";
                out << "mflo $t0\n";
            }
            else
            {
                out << "div $t1, $t0\n";
                out << "mfhi $t0\n";
            }
        }
        return "$t0";
    }

    // exponentiation: **
    string genExponent(const vector<Token> &tokens, int &idx)
    {
        genPrimary(tokens, idx);
        while (idx < (int)tokens.size() && tokens[idx].type == TOKEN_EXP)
        {
            idx++;                      // consume **
            out << "sw $t0, 24($t9)\n"; // save base
            genPrimary(tokens, idx);    // exponent -> $t0
            // naive integer pow loop
            out << "li $t2, 1\n";       // result
            out << "lw $t3, 24($t9)\n"; // base
            string L = newLabel("pow_loop");
            string E = newLabel("pow_end");
            out << L << ":\n";
            out << "beq $t0, $zero, " << E << "\n";
            out << "mul $t2, $t2, $t3\n";
            out << "addi $t0, $t0, -1\n";
            out << "j " << L << "\n";
            out << E << ":\n";
            out << "move $t0, $t2\n";
        }
        return "$t0";
    }

    // primary: NUM | ID | STRING | BOOLEAN | ( expr )
    string genPrimary(const vector<Token> &tokens, int &idx)
    {
        if (idx >= (int)tokens.size())
        {
            out << "li $t0, 0\n";
            return "$t0";
        }
        Token tk = tokens[idx++];
        if (tk.type == TOKEN_NUM)
        {
            out << "li $t0, " << tk.lexeme << "\n";
            return "$t0";
        }
        if (tk.type == TOKEN_ID)
        {
            string lbl = ensureVarLabel(tk.lexeme);
            out << "lw $t0, " << lbl << "\n";
            return "$t0";
        }
        if (tk.type == TOKEN_STRING)
        {
            string lbl = addStringLiteral(tk.lexeme);
            out << "la $t0, " << lbl << "\n";
            return "$t0";
        }
        if (tk.type == TOKEN_BOOLEAN)
        {
            if (strcmp(tk.lexeme, "verdadeiro") == 0 || strcmp(tk.lexeme, "verdade") == 0)
                out << "li $t0, 1\n";
            else
                out << "li $t0, 0\n";
            return "$t0";
        }
        if (tk.type == TOKEN_LPAREN)
        {
            string r = generateExpression(tokens, idx);
            if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_RPAREN)
                idx++;
            return r;
        }
        // fallback
        out << "li $t0, 0\n";
        return "$t0";
    }

    // -------------------- Print --------------------
    // supports concatenation by '+' at top-level; prints strings and integers (simple cases)
    void generatePrint(const vector<Token> &tokens, int &idx)
    {
        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_LPAREN)
            idx++;

        vector<vector<Token>> segments;
        vector<Token> cur;
        int depth = 0;
        while (idx < (int)tokens.size() && !(tokens[idx].type == TOKEN_RPAREN || tokens[idx].type == TOKEN_SEMICOLON))
        {
            Token tk = tokens[idx];
            if (tk.type == TOKEN_PLUS && depth == 0)
            {
                segments.push_back(cur);
                cur.clear();
                idx++;
                continue;
            }
            if (tk.type == TOKEN_LPAREN)
            {
                depth++;
                cur.push_back(tk);
                idx++;
                continue;
            }
            if (tk.type == TOKEN_RPAREN)
            {
                depth--;
                cur.push_back(tk);
                idx++;
                continue;
            }
            cur.push_back(tk);
            idx++;
        }
        if (!cur.empty())
            segments.push_back(cur);

        for (auto &seg : segments)
        {
            if (seg.size() == 1 && seg[0].type == TOKEN_STRING)
            {
                string lbl = addStringLiteral(seg[0].lexeme);
                out << "la $a0, " << lbl << "\n";
                out << "li $v0, 4\nsyscall\n";
            }
            else if (seg.size() >= 1)
            {
                // support simple single-token segments: NUM, ID, BOOLEAN
                Token first = seg[0];
                if (first.type == TOKEN_NUM)
                {
                    out << "li $t0, " << first.lexeme << "\n";
                }
                else if (first.type == TOKEN_ID)
                {
                    string lbl = ensureVarLabel(first.lexeme);
                    out << "lw $t0, " << lbl << "\n";
                }
                else if (first.type == TOKEN_BOOLEAN)
                {
                    if (strcmp(first.lexeme, "verdadeiro") == 0 || strcmp(first.lexeme, "verdade") == 0)
                        out << "li $t0,1\n";
                    else
                        out << "li $t0,0\n";
                }
                else
                {
                    // fallback: evaluate the segment by writing tokens into a tiny evaluator would be complex;
                    // for safety we set 0
                    out << "li $t0, 0\n";
                }
                // print integer
                out << "move $a0, $t0\n";
                out << "li $v0, 1\nsyscall\n";
            }
        }

        if (idx < (int)tokens.size() && tokens[idx].type == TOKEN_RPAREN)
            idx++;
    }
};

#endif // CODEGEN_H
