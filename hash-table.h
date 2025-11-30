
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <iostream>
#include <cstring>
#include <limits.h>
#include <climits>

using namespace std;

// ---------------- SYMBOL INFO --------------------

struct SymbolInfo
{
    char name[64]; // identificador
    char type[32]; // inteiro, logico, caractere
    int tokenType; // ID, NUM, STRING, TIPO_INT...
    int lineDeclared;

    int sizeInBytes;         // 4 = inteiro, 1 = char, 1 = booleano
    unsigned char memory[8]; // representação Big-endian
};

// ---------------- HASH MAP --------------------
// Linked List node
struct node
{
    SymbolInfo info;
    node *next;
};

struct hashMap
{
    int capacity;
    node **arr;
};

// like constructor
void initializeHashMap(struct hashMap *mp)
{
    // Default capacity in this case
    mp->capacity = 101;
    mp->arr = (node **)malloc(sizeof(node *) * mp->capacity);

    for (int i = 0; i < mp->capacity; i++)
        mp->arr[i] = NULL;

    return;
}

int hashFunction(struct hashMap *mp, const char *key)
{
    long long hash = 0, p = 31;
    long long pow = 1;

    for (int i = 0; key[i]; i++)
    {
        long long val = tolower(key[i]);

        // evita caracteres fora do alfabeto
        val = (val >= 'a' && val <= 'z') ? (val - 'a' + 1) : (val + 1);

        hash = (hash + val * pow) % mp->capacity;
        pow = (pow * p) % mp->capacity;
    }

    if (hash < 0)
        hash += mp->capacity;

    return (int)hash;
}

// ---------------- CREATE SYMBOL INFO --------------------

SymbolInfo createSymbol(const char *name, const char *type, int tokenType, int line)
{
    SymbolInfo s;

    strcpy(s.name, name);
    strcpy(s.type, type);
    s.tokenType = tokenType;
    s.lineDeclared = line;

    // Define tamanho e memória em Big-endian
    if (strcmp(type, "inteiro") == 0)
    {
        s.sizeInBytes = 4;
        s.memory[0] = s.memory[1] = s.memory[2] = s.memory[3] = 0;
    }
    else if (strcmp(type, "caractere") == 0)
    {
        s.sizeInBytes = 1;
        s.memory[0] = 0;
    }
    else if (strcmp(type, "logico") == 0)
    {
        s.sizeInBytes = 1;
        s.memory[0] = 0; // 0000 0000 = Mentira
    }

    return s;
}

// ---------------- INSERT --------------------

void insertSymbol(hashMap *mp, SymbolInfo s)
{
    int idx = hashFunction(mp, s.name);

    node *newNode = (node *)malloc(sizeof(node));
    newNode->info = s;
    newNode->next = mp->arr[idx];
    mp->arr[idx] = newNode;
}

// ---------------- SEARCH --------------------

SymbolInfo *searchSymbol(hashMap *mp, const char *name)
{
    int idx = hashFunction(mp, name);
    node *head = mp->arr[idx];

    while (head)
    {
        if (strcmp(head->info.name, name) == 0)
            return &head->info;
        head = head->next;
    }

    return NULL; // not found
}

// ---------------- VALUE ASSIGNMENT (BIG-ENDIAN) --------------------

void storeInteger(SymbolInfo *s, int value)
{
    s->memory[0] = (value >> 24) & 0xFF;
    s->memory[1] = (value >> 16) & 0xFF;
    s->memory[2] = (value >> 8) & 0xFF;
    s->memory[3] = (value) & 0xFF;
}

void storeChar(SymbolInfo *s, char c)
{
    s->memory[0] = c; // 1 byte ASCII
}

void storeBoolean(SymbolInfo *s, bool b)
{
    s->memory[0] = b ? 0xFF : 0x00; // 1111 1111 ou 0000 0000
}

#endif