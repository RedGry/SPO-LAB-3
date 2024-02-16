//
// Created by trott on 15.02.2024.
//

#ifndef LAB_3_ASM_GENERATOR_H
#define LAB_3_ASM_GENERATOR_H

#include "../ast/preprocess_ast.h"
#include "../analyser/builtin_functions.h"

extern int labelCounter;
extern FILE *asmCodeOut;
extern FILE *asmDataOut;

int generate_asm(preparedFunc *funcs, int count, builtinFunctions functions);

#endif //LAB_3_ASM_GENERATOR_H
