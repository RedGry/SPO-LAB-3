//
// Created by trott on 15.02.2024.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "symbolic_table.h"
#include "builtin_functions.h"

symbolicTable *newSymbolicTable(symbolicTable *parent) {
    symbolicTable *table = malloc(sizeof(symbolicTable));
    table->parent = parent;
    table->symbolsCount = 0;
    table->capacity = 5;
    table->symbols = malloc(sizeof(symbol) * table->capacity);
    return table;
}

void symbolicTable_free(symbolicTable *table) {
    free(table->symbols);
    free(table);
}

symbol *symbolicTable_findSymbol(symbolicTable *table, char *identifier) {
//    if (table == NULL) {
//        printf("GGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n");
//    } else {
//        printf("GGGGGGGGGGGGGGGGGGGGGGGGGGGGG2\n");
//    }

    if (table == NULL) {
        return NULL;
    }

    printf("start-finder\n");

    for (int i = 0; i < table->symbolsCount; ++i) {
//        printf(table->symbols[i].identifier);
//        printf(" , ");
//        printf(identifier);
//        printf("\n");
        if (strcmp(table->symbols[i].identifier, identifier) == 0) {
            return &table->symbols[i];
        }
    }

    return symbolicTable_findSymbol(table->parent, identifier);
}


char *symbolicTable_currentFuncId(symbolicTable *table) {
    if (table == NULL) {
        return NULL;
    }
    if (table->currentFuncId == NULL) {
        return symbolicTable_currentFuncId(table->parent);
    }
    return table->currentFuncId;
}

int symbolicTable_putSymbol(symbolicTable *table, preparedType type, char *identifier, char *label, union ctx ctx,
                            enum symbolCategory category) {
    printf("symbolicTable_putSymbol\n");
    if (identifier == NULL) {
        fprintf(stderr, "identifier is NULL\n");
        return 1;
    }
    printf("start if\n");
    if (table->symbolsCount >= table->capacity) {
        fprintf(stderr, "table capacity exceed, double it: %d -> %d\n", table->capacity, table->capacity * 2);
        table->capacity *= 2;
        symbol *s = malloc(table->capacity * sizeof(symbol) + 10);
        for (int i = 0; i < table->symbolsCount; ++i) {
            s[i] = table->symbols[i];
        }
        free(table->symbols);
        table->symbols = s;
    }
    printf("after if\n");
//    printf(identifier);
//    printf(" - ");
    identifier = strToHeap(identifier);
//    printf(identifier);
//    printf("\n");
    symbol *s = symbolicTable_findSymbol(table, identifier);
    if (s != NULL) {
        fprintf(stderr, "symbol %s already exists\n", identifier);
        return 3;
    }
    printf("init table\n");
    table->symbols[table->symbolsCount].type = type;
    table->symbols[table->symbolsCount].identifier = identifier;
    table->symbols[table->symbolsCount].ctx = ctx;
    table->symbols[table->symbolsCount].category = category;
    table->symbols[table->symbolsCount].label = label;

    table->symbolsCount++;
    return 0;
}
