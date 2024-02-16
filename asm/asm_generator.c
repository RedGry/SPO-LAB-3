//
// Created by trott on 15.02.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asm_generator.h"
#include "../analyser/symbolic_table.h"
#include "../analyser/builtin_functions.h"

#define asm_code_header "[section code_ram]\n\tjump start\n"
#define asm_data_header "[section data_ram]\n"
#define asm_footer "halt:\n\thlt\n"

#define mnemonic_1(mnemonic, arg1) fprintf(asmCodeOut, "\t%s %s\n", mnemonic, arg1);
#define mnemonic_2(mnemonic, arg1, arg2) fprintf(asmCodeOut, "\t%s %s, %s\n", mnemonic, arg1, arg2);
#define mnemonic_2_i(mnemonic, arg1, arg2) fprintf(asmCodeOut, "\t%s %d, %s\n", mnemonic, arg1, arg2);
#define mnemonic_3(mnemonic, arg1, arg2, arg3) fprintf(asmCodeOut, "\t%s %s, %s, %s\n", mnemonic, arg1, arg2, arg3);
#define mnemonic_0(mnemonic) fprintf(asmCodeOut, "\t%s\n", mnemonic);

#define mov_i(from, to) mnemonic_2_i("mov", from, to)
#define push(reg) mnemonic_1("push", reg)
#define pop(reg) mnemonic_1("pop", reg)
#define add(op1, op2, to) mnemonic_3("add", op1, op2, to)
#define sub(op1, op2, to) mnemonic_3("sub", op1, op2, to)
#define mul(op1, op2, to) mnemonic_3("mul", op1, op2, to)
#define div(op1, op2, to) mnemonic_3("div", op1, op2, to)
#define rem(op1, op2, to) mnemonic_3("rem", op1, op2, to)
#define and(op1, op2, to) mnemonic_3("_and", op1, op2, to)
#define or(op1, op2, to) mnemonic_3("_or", op1, op2, to)
#define neg(op, to) mnemonic_2("neg", op, to)
#define not(op, to) mnemonic_2("_not", op, to)

#define jumpeq(op, to) mnemonic_2("jumpeq", op, to)
#define jumpgt(op, to) mnemonic_2("jumpgt", op, to)
#define jumpge(op, to) mnemonic_2("jumpge", op, to)
#define jumplt(op, to) mnemonic_2("jumplt", op, to)
#define jumple(op, to) mnemonic_2("jumple", op, to)
#define jump(to) mnemonic_1("jump", to)

#define load(from, to) mnemonic_2("load", from, to)
#define store(from, to) mnemonic_2("store", from, to)

#define ret() mnemonic_0("ret")
#define call(label) mnemonic_1("call", label)

FILE *asmCodeOut;
FILE *asmDataOut;

char *labelName() {
    char *c = malloc(sizeof(char) * 32);
    sprintf(c, "label_%d", labelCounter++);
    return c;
}

char *labelNameIdent(char *ident) {
    char *c = malloc(sizeof(char) * 32);
    sprintf(c, "%s_%d", ident, labelCounter++);
    return c;
}

#define put_label(name) fprintf(asmCodeOut, "%s:\n", name);
#define put_comment(comment) fprintf(asmCodeOut, "\t\t;%s\n", comment);
#define put_comment_var(comment) fprintf(asmDataOut, "\t\t;%s\n", comment);
#define put_label_var(name) fprintf(asmDataOut, "%s: dw 0x0\n", name);


int translate_literal(preparedLiteral literal) {
    switch (literal.type.type) {
        case INT:
            mov_i(literal.i_value, "r0")
            break;
        case BOOL:
            if (literal.b_value == BOOLEAN_TRUE) {
                mov_i(1, "r0")
            } else {
                mov_i(0, "r0")
            }
            break;
    }
    return 0;
}

int translate_expression(preparedExpression expression, symbolicTable *table);

int translate_binary(preparedBinary binary, symbolicTable *table) {
    translate_expression(*binary.leftOperand, table);
    translate_expression(*binary.rightOperand, table);
    pop("r1");
    pop("r0");
    switch (binary.type) {
        case BINARY_TYPE_PLUS:
            add("r0", "r1", "r0")
            break;
        case BINARY_TYPE_MINUS:
            sub("r0", "r1", "r0")
            break;
        case BINARY_TYPE_STAR:
            mul("r0", "r1", "r0")
            break;
        case BINARY_TYPE_SLASH:
            div("r0", "r1", "r0")
            break;
        case BINARY_TYPE_PERCENT:
            rem("r0", "r1", "r0")
            break;
        case BINARY_TYPE_EQUAL: {
            sub("r0", "r1", "r0") // a - b > 0
            char *label1 = labelName();
            char *label2 = labelName();
            jumpeq("r0", label1)
            mov_i(0, "r0")
            jump(label2)
            put_label(label1)
            mov_i(1, "r0")
            put_label(label2)
            free(label1);
            free(label2);
        }
            break;
        case BINARY_TYPE_NOTEQUAL: {
            sub("r0", "r1", "r0") // a - b != 0
            char *label1 = labelName();
            char *label2 = labelName();
            jumpeq("r0", label1)
            mov_i(1, "r0")
            jump(label2)
            put_label(label1)
            mov_i(0, "r0")
            put_label(label2)
            free(label1);
            free(label2);
        }
            break;
        case BINARY_TYPE_LESSTHAN: // a < b
        {
            sub("r0", "r1", "r0") // a - b > 0
            char *label1 = labelName();
            char *label2 = labelName();
            jumplt("r0", label1)
            mov_i(0, "r0")
            jump(label2)
            put_label(label1)
            mov_i(-1, "r0")
            put_label(label2)
            free(label1);
            free(label2);
        }
            break;
        case BINARY_TYPE_GREATERTHAN: // a > b
        {
            sub("r0", "r1", "r0") // a - b > 0
            char *label1 = labelName();
            char *label2 = labelName();
            jumpgt("r0", label1)
            mov_i(0, "r0")
            jump(label2)
            put_label(label1)
            mov_i(-1, "r0")
            put_label(label2)
            free(label1);
            free(label2);
        }
            break;
        case BINARY_TYPE_LESSTHANEQ: // a <= b
        {
            sub("r0", "r1", "r0") // a - b > 0
            char *label1 = labelName();
            char *label2 = labelName();
            jumple("r0", label1)
            mov_i(0, "r0")
            jump(label2)
            put_label(label1)
            mov_i(-1, "r0")
            put_label(label2)
            free(label1);
            free(label2);
        }
            break;
        case BINARY_TYPE_GREATERTHANEQ: // a >= b
        {
            sub("r0", "r1", "r0") // a - b > 0
            char *label1 = labelName();
            char *label2 = labelName();
            jumpge("r0", label1)
            mov_i(0, "r0")
            jump(label2)
            put_label(label1)
            mov_i(-1, "r0")
            put_label(label2)
            free(label1);
            free(label2);
        }
            break;
        case BINARY_TYPE_AND:
            and("r0", "r1", "r0")
            break;
        case BINARY_TYPE_OR:
            or("r0", "r1", "r0")
            break;
    }
    return 0;
}

int translate_place(char *identifier, symbolicTable *table) {
    symbol *s = symbolicTable_findSymbol(table, identifier);
    if (s == NULL) {
        return 1;
    }
    load(s->label, "r0")
    return 0;
}

int labelCounter = 0;

int translate_call(preparedCall call, symbolicTable *table) {
    symbol *s = symbolicTable_findSymbol(table, call.procedureName);
    for (int i = 0; i < call.argumentExpressions.expressionsCount; ++i) {
        translate_expression(call.argumentExpressions.expressions[i], table);
        pop("r0")
        store("r0", s->ctx.func->args.vars[i].label)
    }
    call(s->label)
    return 0;
}

int translate_return(preparedExpression expression, symbolicTable *table) {
    if (!(expression.type == LITERAL && expression.literal.type.type == VOID)) {
        translate_expression(expression, table);
        pop("r0")
    }
    ret()
    return 0;
}

int translate_unary(preparedUnary unary, symbolicTable *table) {
    translate_expression(*unary.operand, table);
    pop("r0");
    switch (unary.type) {
        case UNARY_TYPE_NOT:
            not("r0", "r0")
            break;
        case UNARY_TYPE_MINUS:
            neg("r0", "r0")
            break;
    }
    return 0;
}

int translate_expression(preparedExpression expression, symbolicTable *table) {
    int res = 0;
    switch (expression.type) {
        case UNARY:
            res = translate_unary(expression.unary, table);
            break;
        case BINARY:
            res = translate_binary(expression.binary, table);
            break;
        case BRACES:
            return translate_expression(*expression.expression, table); // no need to push already pushed r0
        case CALL:
            res = translate_call(expression.call, table);
            break;
        case INDEXER:
            break;
        case PLACE:
            res = translate_place(expression.identifier, table);
            break;
        case LITERAL:
            res = translate_literal(expression.literal);
            break;
    }
    push("r0");
    return res;
}

int translate_variable(preparedVar var, symbolicTable *table) {
    char *label;
    if (var.label != NULL) {
        label = var.label;
    } else {
        label = labelName();
    }
    union ctx ctx = {};
    if (symbolicTable_putSymbol(table, var.type, var.identifier, label, ctx, SYMBOL_CATEGORY_VAR) != 0) {
        return 1;
    }
    put_label_var(label)

    if (var.isInitValueExists > 0) {
        translate_expression(*var.initValue, table);
        pop("r0")
        store("r0", label)
    }

    return 0;
}

int translate_block(preparedBlock block, symbolicTable *table, char *lastGoThroughLabel);

int translate_statement(preparedStatement statement, symbolicTable *table, char *lastGoThroughLabel);

int translate_ifs(preparedIf ifs, symbolicTable *table, char *lastGoThroughLabel) {
    put_comment("if")
    put_comment(ifs.statement.condition.astNode->value)
    if (translate_expression(ifs.statement.condition, table) != 0) { // r0 contains bool expression
        return 1;
    };
    char *goThrough = labelName();
    char *elseLabel;
    pop("r0")
    if (ifs.elseStatementExists == 1) {
        elseLabel = labelName();
        jumpeq("r0", elseLabel) // if r0 == 0 goto else conditional statement
    } else {
        jumpeq("r0", goThrough) // if r0 == 0 goto else conditional statement
    }
    put_comment("then")
    if (translate_statement(ifs.statement.statement, table, lastGoThroughLabel) != 0) {
        return 1;
    }
    if (ifs.elseStatementExists == 1) {
        jump(goThrough)
        put_label(elseLabel)
        put_comment("else")
        if (translate_statement(ifs.elseStatement, table, lastGoThroughLabel) != 0) {
            return 1;
        }
    }
    put_comment("endif")
    put_label(goThrough)
    return 0;
}

int translate_while(preparedWhile whilep, symbolicTable *table) {
    put_comment("while")
    char *loopbackLabel = labelName();
    put_label(loopbackLabel)
    put_comment(whilep.condition.astNode->value)
    if (translate_expression(whilep.condition, table) != 0) { // r0 contains bool expression
        return 1;
    };
    char *goThrough = labelName();
    pop("r0")
    jumpeq("r0", goThrough) // if r0 == 0 goto else conditional statement
    put_comment("while body")
    if (translate_block(whilep.block, table, goThrough) != 0) {
        return 1;
    }
    jump(loopbackLabel)
    put_label(goThrough)
    put_comment("end while")
    return 0;
}

int translate_dowhile(preparedDoWhile doWhile, symbolicTable *table) {
    put_comment("doWhile")
    char *loopbackLabel = labelName();
    put_label(loopbackLabel)

    char *goThrough = labelName();

    put_comment("doWhile body")
    if (translate_block(doWhile.block, table, goThrough) != 0) {
        return 1;
    }
    put_comment("doWhile condition")
    put_comment(doWhile.condition.astNode->value)
    if (translate_expression(doWhile.condition, table) != 0) { // r0 contains bool expression
        return 1;
    };
    pop("r0") // get result of expression
    not("r0", "r0") // invert
    jumpeq("r0", loopbackLabel) // if result of expression is true goto start of do while body
    put_label(goThrough)
    put_comment("end doWhile")
    return 0;
}

int translate_assigment(preparedAssigment assigment, symbolicTable *table) {
    symbol *s = symbolicTable_findSymbol(table, assigment.to.identifier);
    if (s == NULL) {
        return 1;
    }
    translate_expression(*assigment.rightPart, table);
    pop("r0")
    store("r0", s->label)
    return 0;
}

int translate_statement(preparedStatement statement, symbolicTable *table, char *lastGoThroughLabel) {
    switch (statement.type) {
        case STATEMENT_TYPE_BLOCK:
            return translate_block(statement.block, table, lastGoThroughLabel);
        case STATEMENT_TYPE_VAR:
            for (int i = 0; i < statement.vars.count; ++i) {
                if (translate_variable(statement.vars.vars[i], table) != 0) {
                    return 1;
                }
            }
            break;
        case STATEMENT_TYPE_IF:
            return translate_ifs(*statement.ifp, table, lastGoThroughLabel);
        case STATEMENT_TYPE_WHILE:
            return translate_while(statement.whilep, table); // don't pass lastGoThroughLabel
        case STATEMENT_TYPE_DO_WHILE:
            return translate_dowhile(statement.dowhile, table);
        case STATEMENT_TYPE_BREAK:
            if (lastGoThroughLabel == NULL) {
                fprintf(stderr, "unexpected break\n");
                return 1;
            }
            put_comment("break")
            jump(lastGoThroughLabel)
            break;
        case STATEMENT_TYPE_EXPRESSION:
            return translate_expression(statement.expression, table);
        case STATEMENT_TYPE_ASSIGMENT:
            return translate_assigment(statement.assigment, table);
        case STATEMENT_TYPE_RETURN:
            return translate_return(statement.expression, table);
    }
    return 0;
}

int translate_block(preparedBlock block, symbolicTable *table, char *lastGoThroughLabel) {
    for (int i = 0; i < block.statementsCount; ++i) {
        if (translate_statement(block.statements[i], table, lastGoThroughLabel) != 0) {
            return 1;
        }
    }
    return 0;
}

int translate_builtin_procedure(builtinFunction function, symbolicTable *table) {
    printf(function.func.identifier);
    printf("\n");
    symbol *s = symbolicTable_findSymbol(table, function.func.identifier);

    put_label(s->label)
    for (int i = 0; i < function.func.args.count; ++i) {
        if (translate_variable(function.func.args.vars[i], table) != 0) {
            return 1;
        }
    }
    fprintf(asmCodeOut, "%s\n", function.asmBody);
    return 0;
}

int translate_procedure(int num, preparedFunc *funcs, int count, symbolicTable *table) {
    table = newSymbolicTable(table);
    symbol *s = symbolicTable_findSymbol(table, funcs[num].identifier);
    preparedFunc func = funcs[num];
    put_label(s->label)
    for (int i = 0; i < func.args.count; ++i) {
        if (translate_variable(func.args.vars[i], table) != 0) {
            return 1;
        }
    }
    if (translate_block(func.body, table, NULL) != 0) {
        return 1;
    }
    ret()
    return 0;
}

int generate_asm(preparedFunc *funcs, int count, builtinFunctions functions) {
    symbolicTable *table = newSymbolicTable(NULL);

    for (int i = 0; i < count; ++i) {
        if (funcs[i].seen == 0) {
            continue;
        }
        union ctx ctx = {&funcs[i]};
        for (int j = 0; j < funcs[i].args.count; ++j) {
            funcs[i].args.vars[j].label = labelNameIdent(funcs[i].identifier);
        }

        if (symbolicTable_putSymbol(table, funcs[i].returnType, funcs[i].identifier, labelNameIdent(funcs[i].identifier), ctx,
                                    SYMBOL_CATEGORY_FUNC) != 0) {
            return 1;
        }
    }
    fprintf(asmCodeOut, asm_code_header);
    fprintf(asmDataOut, asm_data_header);

    for (int i = 0; i < functions.count; ++i) {
        if (functions.functions[i].func.seen == 0) {
            continue;
        }
        union ctx ctx = {
                &functions.functions[i].func
        };
        for (int j = 0; j < functions.functions[i].func.args.count; ++j) {
            if (functions.functions[i].func.args.vars[i].label == NULL) {
                functions.functions[i].func.args.vars[i].label = labelNameIdent(functions.functions[i].func.identifier);
            }
        }

        if (symbolicTable_putSymbol(table, functions.functions[i].func.returnType,
                                    functions.functions[i].func.identifier,
                                    labelNameIdent(functions.functions[i].func.identifier), ctx,
                                    SYMBOL_CATEGORY_FUNC) != 0) {
            return 1;
        }

        if (translate_builtin_procedure(functions.functions[i], table) != 0) {
            return 1;
        }
    }

    for (int i = 0; i < count; ++i) {
        if (funcs[i].seen == 0) {
            continue;
        }
        if (strcmp(funcs[i].identifier, "main") == 0) {
            put_label("start")
        }
        if (translate_procedure(i, funcs, count, table) != 0) {
            return 2;
        }
        if (strcmp(funcs[i].identifier, "main") == 0) {
            jump("halt")
        }
    }
    fprintf(asmCodeOut, asm_footer);

    return 0;
}

