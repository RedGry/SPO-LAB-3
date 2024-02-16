//
// Created by trott on 15.02.2024.
//

#ifndef LAB_3_BUILTIN_FUNCTIONS_H
#define LAB_3_BUILTIN_FUNCTIONS_H

#include "../ast/preprocess_ast.h"

typedef struct builtinFunction builtinFunction;
typedef struct builtinFunctions builtinFunctions;
#define countOfBuiltinFunctions 2

struct builtinFunction {
    preparedFunc func;
    const char *asmBody;
};

struct builtinFunctions {
    builtinFunction *functions;
    int count;
};
char* strToHeap(char* s);

builtinFunctions getBuiltinFunctions();


#endif //LAB_3_BUILTIN_FUNCTIONS_H
