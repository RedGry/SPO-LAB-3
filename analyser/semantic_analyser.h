//
// Created by trott on 15.02.2024.
//

#ifndef LAB_3_SEMANTIC_ANALYSER_H
#define LAB_3_SEMANTIC_ANALYSER_H

#include "../ast/preprocess_ast.h"
#include "builtin_functions.h"

int processSemantics(preparedFunc *funcs, int count, builtinFunctions functions);


#endif //LAB_3_SEMANTIC_ANALYSER_H
