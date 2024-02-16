//
// Created by trott on 15.02.2024.
//

#include <stdlib.h>
#include <string.h>
#include "builtin_functions.h"

char* strToHeap(char* s) {
    int len = strlen(s);
    // Выделяем память на 1 байт больше для нуль-терминатора
    char* r = malloc((len + 1) * sizeof(char));
    if (r == NULL) {
        // Обработка ошибки выделения памяти
        return NULL;
    }
    for (int i = 0; i < len; ++i) {
        r[i] = s[i];
    }
    // Добавляем нуль-терминатор в конец строки
    r[len] = '\0';
    return r;
}

builtinFunctions getBuiltinFunctions() {
    builtinFunctions functions;
    functions.functions = malloc(sizeof(builtinFunction) * countOfBuiltinFunctions);
    functions.count = countOfBuiltinFunctions;

    preparedVar *writeArgs = malloc(sizeof(preparedVar));
    writeArgs[0].identifier = strToHeap("val");
    writeArgs[0].type.type = INT;
    writeArgs[0].type.arrayDem = 0;
    writeArgs[0].type.customTypeIdentifier = strToHeap("int");
    writeArgs[0].label = strToHeap("label_write_val");
    writeArgs[0].isInitValueExists = 0;


    functions.functions[0].func.identifier = strToHeap("read");
    functions.functions[0].func.args.count = 0;
    functions.functions[0].func.returnType.type = INT;
    functions.functions[0].func.returnType.arrayDem = 0;
    functions.functions[0].func.returnType.customTypeIdentifier =strToHeap( "int");
    functions.functions[0].asmBody = strToHeap("\tmov inReg, r0\n"
                                               "\tret\n");

    functions.functions[1].func.identifier = strToHeap("write");
    functions.functions[1].func.args.vars = writeArgs;
    functions.functions[1].func.args.count = 1;
    functions.functions[1].func.returnType.type = VOID;
    functions.functions[1].func.returnType.arrayDem = 0;
    functions.functions[1].func.returnType.customTypeIdentifier = strToHeap("int");
    functions.functions[1].asmBody = strToHeap("\tload label_write_val, outReg\n"
                                               "\tret\n");

    return functions;
}

