//
// Created by trott on 15.02.2024.
//

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "semantic_analyser.h"
#include "symbolic_table.h"
#include "builtin_functions.h"

struct context {
    preparedFunc *funcs;
    int count;
};


int compareTypes(preparedType type1, preparedType type2) {
    if (type1.type != type2.type) {
        fprintf(stderr, "cannot convert %s to %s", primitiveType_toString(type1.type),
                primitiveType_toString(type2.type));
        return 1;
    }
    if (type1.arrayDem != type2.arrayDem) {
        fprintf(stderr, "cannot convert array[%d] to array[%d]", type1.arrayDem, type2.arrayDem);
        return 2;
    }
    if (type1.customTypeIdentifier == NULL) {
        return 0;
    }
    if (strcmp(type1.customTypeIdentifier, type2.customTypeIdentifier) != 0) {
        fprintf(stderr, "cannot convert %s type to %s type", type1.customTypeIdentifier, type2.customTypeIdentifier);
        return 3;
    }
    return 0;
}

int preparedStatement_visit(preparedStatement statement, symbolicTable *table);

int preparedExpression_visit(preparedExpression expression, symbolicTable *table, preparedType *expectedType,
                             preparedType *pType);

int preparedBlock_visit(preparedBlock block, symbolicTable *table) {
    int countVars = 0;
    for (int i = 0; i < block.statementsCount; ++i) {
        if (block.statements[i].type == STATEMENT_TYPE_VAR) {
            countVars += block.statements[i].vars.count;
        }
    }
    table = newSymbolicTable(table);
    for (int i = 0; i < block.statementsCount; ++i) {
        if (preparedStatement_visit(block.statements[i], table) != 0) {
            symbolicTable_free(table);
            return 1;
        }
    }
    symbolicTable_free(table);
    return 0;
}

int preparedVars_visit(preparedVars vars, symbolicTable *table) {
    for (int i = 0; i < vars.count; ++i) {
        if (vars.vars[i].initValue != NULL) {
            if (preparedExpression_visit(*vars.vars[i].initValue, table, &vars.vars[i].type, NULL) != 0) {
                return 2;
            }
        }
        union ctx c = {};
        if (symbolicTable_putSymbol(table, vars.vars[i].type, vars.vars[i].identifier, NULL, c, SYMBOL_CATEGORY_VAR) !=
            0) {
            return 1;
        }
    }
    return 0;
}

int
preparedBinary_visit(preparedBinary binary, symbolicTable *table, preparedType *expectedType, preparedType *foundType) {
    preparedType type = {};
    symbol *s;
    if (expectedType != NULL) {
        switch (binary.type) {
            case BINARY_TYPE_PLUS:
            case BINARY_TYPE_MINUS:
            case BINARY_TYPE_STAR:
            case BINARY_TYPE_SLASH:
            case BINARY_TYPE_PERCENT:
                if (expectedType->type != INT) {
                    fprintf(stderr, "expected int type, got %s", primitiveType_toString(expectedType->type));
                    return 1;
                }
                break;
            case BINARY_TYPE_AND:
            case BINARY_TYPE_OR:
            case BINARY_TYPE_EQUAL:
            case BINARY_TYPE_NOTEQUAL:
            case BINARY_TYPE_LESSTHAN:
            case BINARY_TYPE_GREATERTHAN:
            case BINARY_TYPE_LESSTHANEQ:
            case BINARY_TYPE_GREATERTHANEQ:
                if (expectedType->type != BOOL) {
                    fprintf(stderr, "expected boolean type, got %s", primitiveType_toString(expectedType->type));
                    return 1;
                }
                expectedType = NULL;
                break;
        }
    }
    preparedType foundTypeLeft = {};
    if (preparedExpression_visit(*binary.leftOperand, table, expectedType, &foundTypeLeft) != 0) {
        return 1;
    }
    if (foundType != NULL) {
        *foundType = foundTypeLeft;
    }
    if (expectedType != NULL) {
        expectedType = foundType;
    }
    return preparedExpression_visit(*binary.rightOperand, table, expectedType, NULL);
}

int
preparedUnary_visit(preparedUnary unary, symbolicTable *table, preparedType *expectedType, preparedType *foundType) {
    return preparedExpression_visit(*unary.operand, table, expectedType, foundType);
}

int processSemanticsForFunction(preparedFunc *func, symbolicTable *table);

int preparedExpression_visit(preparedExpression expression, symbolicTable *table, preparedType *expectedType,
                             preparedType *foundType) {

    symbol *s;
    int result = 0;
    switch (expression.type) {
        case UNARY:
            result = preparedUnary_visit(expression.unary, table, expectedType, foundType);
            break;
        case BINARY:
            result = preparedBinary_visit(expression.binary, table, expectedType, foundType);
            break;
        case BRACES:
            result = preparedExpression_visit(*expression.expression, table, expectedType, foundType);
            break;
        case CALL:
            s = symbolicTable_findSymbol(table, expression.call.procedureName);
            if (s == NULL) {
                fprintf(stderr, "function %s not found", expression.call.procedureName);
                return 1;
            }
            if (s->type.type != FUNC || s->category != SYMBOL_CATEGORY_FUNC) {
                fprintf(stderr, "%s is not a function", expression.call.procedureName);
                return 2;
            }
            if (expression.call.argumentExpressions.expressionsCount != s->ctx.func->args.count) {
                fprintf(stderr, "invalid count of call arguments, expected: %d, got: %d",
                        s->ctx.func->args.count,
                        expression.call.argumentExpressions.expressionsCount);
                return 2;
            }
            for (int i = 0; i < expression.call.argumentExpressions.expressionsCount; ++i) {
                preparedExpression_visit(expression.call.argumentExpressions.expressions[i], table,
                                         &s->ctx.func->args.vars[i].type,
                                         NULL);
            }
            if (foundType != NULL) {
                *foundType = s->ctx.func->returnType;
            }
            if (expectedType != NULL) {
                if (compareTypes(s->ctx.func->returnType, *expectedType) != 0) {
                    return 2;
                }
            }
            if (s->ctx.func->seen == 0) {
                if (processSemanticsForFunction(s->ctx.func, table) != 0) {
                    return 1;
                }
            }
            break;
        case INDEXER:
//            return 0;
//            if (expectedType != NULL) {
//                t = *expectedType;
//                t.arrayDem--;
//                type = &t;
//            }
            result = preparedExpression_visit(*expression.indexer.expression, table, NULL, foundType);
            break;
        case PLACE:
            s = symbolicTable_findSymbol(table, expression.identifier);
            if (s == NULL) {
                fprintf(stderr, "symbol %s is not defined", expression.identifier);
                return 1;
            }
            if (s->category != SYMBOL_CATEGORY_VAR) {
                fprintf(stderr, "%s is not a variable", expression.identifier);
                return 3;
            }
            if (expectedType != NULL) {
                if (compareTypes(s->type, *expectedType) != 0) {
                    fprintf(stderr, "%s incorrect type", expression.identifier);
                    return 2;
                }
            }
            if (foundType != NULL) {
                *foundType = s->type;
            }
            break;
        case LITERAL:
            if (expectedType != NULL) {
                if (compareTypes(expression.literal.type, *expectedType) != 0) {
                    return 2;
                }
            }
            if (foundType != NULL) {
                *foundType = expression.literal.type;
            }
            break;
    }
    if (result != 0) { return result; }

    return result;
}

preparedType bool = {.type =  BOOL};

int preparedWhile_visit(preparedWhile whilep, symbolicTable *table) {
    if (preparedExpression_visit(whilep.condition, table, &bool, NULL) != 0) {
        return 1;
    }
    return preparedBlock_visit(whilep.block, table);
}

int preparedDoWhile_visit(preparedDoWhile dowhile, symbolicTable *table) {
    if (preparedExpression_visit(dowhile.condition, table, &bool, NULL) != 0) {
        return 1;
    }
    return preparedBlock_visit(dowhile.block, table);
}

int preparedIf_visit(preparedIf *ifp, symbolicTable *table) {
    if (preparedExpression_visit(ifp->statement.condition, table, &bool, NULL) != 0) {
        return 1;
    }
    if (preparedStatement_visit(ifp->statement.statement, table) != 0) {
        return 1;
    }
    if (ifp->elseStatementExists == 1) {
        if (preparedStatement_visit(ifp->elseStatement, table) != 0) {
            return 1;
        }
    }
    return 0;
}

int preparedAssigment_visit(preparedAssigment assigment, symbolicTable *table) {
    symbol *s;
    switch (assigment.to.type) {
        case PLACE:
            s = symbolicTable_findSymbol(table, assigment.to.identifier);
            if (s == NULL) {
                return 3;
            }
            return preparedExpression_visit(*assigment.rightPart, table, &s->type, NULL);
        case INDEXER:
            printf("assigment to indexable entities not supported yet");
            return 2;
        default:
            return 1;
    }
    return 0;
}

int preparedStatement_visit(preparedStatement statement, symbolicTable *table) {
    char *funcId;
    symbol *s;
    switch (statement.type) {
        case STATEMENT_TYPE_ASSIGMENT:
            return preparedAssigment_visit(statement.assigment, table);
        case STATEMENT_TYPE_BLOCK:
            return preparedBlock_visit(statement.block, table);
        case STATEMENT_TYPE_VAR:
            return preparedVars_visit(statement.vars, table);
        case STATEMENT_TYPE_IF:
            return preparedIf_visit(statement.ifp, table);
        case STATEMENT_TYPE_WHILE:
            return preparedWhile_visit(statement.whilep, table);
        case STATEMENT_TYPE_DO_WHILE:
            return preparedDoWhile_visit(statement.dowhile, table);
        case STATEMENT_TYPE_BREAK:
            // todo check if we in loop
            break;
        case STATEMENT_TYPE_EXPRESSION:
            return preparedExpression_visit(statement.expression, table, NULL, NULL);
        case STATEMENT_TYPE_RETURN:
            funcId = symbolicTable_currentFuncId(table);
            if (funcId == NULL) {
                return 1;
            }
            s = symbolicTable_findSymbol(table, funcId);
            if (s == NULL) {
                return 2;
            }
            return preparedExpression_visit(statement.expression, table, &s->ctx.func->returnType, NULL);
    }
    return 0;
}

symbolicTable *getGrandparentTable(symbolicTable *table) {
    if (table->parent == NULL) {
        return table;
    }
    return getGrandparentTable(table->parent);
}

int processSemanticsForFunction(preparedFunc *func, symbolicTable *table) {
    printf("processSemanticsForFunction\n");
    func->seen = 1;
    table = getGrandparentTable(table);
    table = newSymbolicTable(table);
    table->currentFuncId = func->identifier;
    for (int i = 0; i < func->args.count; ++i) {
        union ctx c = {};
        if (symbolicTable_putSymbol(table, func->args.vars[i].type, func->args.vars[i].identifier, NULL, c,
                                    SYMBOL_CATEGORY_VAR) != 0) {
            symbolicTable_free(table);
            return 1;
        }
    }
    if (preparedBlock_visit(func->body, table) != 0) {
        symbolicTable_free(table);
        return 1;
    }
    symbolicTable_free(table);
    return 0;
}


int processSemantics(preparedFunc *funcs, int count, builtinFunctions functions) {
    printf("processSemantics\n");
    struct context ctx = {funcs, count};
    symbolicTable *table = newSymbolicTable(NULL);
    preparedFunc *main = NULL;

    printf("for-1\n");
    for (int i = 0; i < functions.count; i++) {
        preparedType type;
        type.type = FUNC;
        union ctx c = {};
        c.func = &functions.functions[i].func;
        c.func->seen = 0;
        if (symbolicTable_putSymbol(table, type, c.func->identifier, NULL, c, SYMBOL_CATEGORY_FUNC) != 0) {
            return 1;
        }
    }
    printf("for-2\n");
    for (int i = 0; i < count; ++i) {
        preparedType type;
        type.type = FUNC;
        union ctx c = {};
        c.func = &funcs[i];
        funcs[i].seen = 0;
        printf("init type and load\n");
        printf("funcs: %s\n", ctx.funcs[i].identifier);
        if (ctx.funcs[i].identifier != NULL) {
            if (strcmp(ctx.funcs[i].identifier, "main") == 0) {
                printf("main set funcs\n");
                main = &ctx.funcs[i];
            }
            printf("processSemantics -> symbolicTable_putSymbol\n");
            if (symbolicTable_putSymbol(table, type, funcs[i].identifier, NULL, c, SYMBOL_CATEGORY_FUNC) != 0) {
                return 1;
            }
        }
    }
    printf("main == NULL\n");
    if (main == NULL) {
        return 1;
    }
    if (main->args.count != 0) {
        fprintf(stderr, "main function cannot have arguments");
    }
    if (main->returnType.type != VOID) {
        fprintf(stderr, "main function should have 'void' return type");
    }
    if (processSemanticsForFunction(main, table) != 0) {
        return 2;
    }
    return 0;
}

