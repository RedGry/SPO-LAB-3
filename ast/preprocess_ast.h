//
// Created by trott on 15.02.2024.
//

#ifndef LAB_3_PREPROCESS_AST_H
#define LAB_3_PREPROCESS_AST_H

#include "ast.h"

typedef struct preparedStatement preparedStatement;
typedef struct preparedUnary preparedUnary;
typedef struct preparedExpression preparedExpression;
typedef struct preparedBlock preparedBlock;
typedef struct preparedBinary preparedBinary;
typedef struct preparedCall preparedCall;
typedef struct preparedIndexer preparedIndexer;
typedef struct expressionsList expressionsList;
typedef struct preparedLiteral preparedLiteral;
typedef struct preparedType preparedType;
typedef struct conditionalStatement conditionalStatement;
typedef struct preparedIf preparedIf;
typedef struct preparedWhile preparedWhile;
typedef struct preparedDoWhile preparedDoWhile;
typedef struct preparedVars preparedVars;
typedef struct preparedVar preparedVar;
typedef struct preparedFunc preparedFunc;
typedef struct preparedAssigment preparedAssigment;

typedef enum statementType statementType;
typedef enum expressionType expressionType;
typedef enum binaryType binaryType;
typedef enum unaryType unaryType;
typedef enum primitiveType primitiveType;
typedef enum BOOLEAN BOOLEAN;


enum primitiveType {
    CHARACTER, INT, BOOL, VOID, CUSTOM, FUNC, RESERVED
};

const char *primitiveType_toString(primitiveType type);

struct preparedType {
    primitiveType type;
    char *customTypeIdentifier;
    int arrayDem;
    ASTNode *astNode;
};
enum BOOLEAN {
    BOOLEAN_TRUE,
    BOOLEAN_FALSE
};
struct preparedLiteral {
    preparedType type;
    char c_value;
    int i_value;
    char *s_value;
    BOOLEAN b_value;
    ASTNode *astNode;
};
struct preparedVar {
    preparedType type;
    char *identifier;
    preparedExpression *initValue;
    int isInitValueExists;
    ASTNode *astNode;

    // for function args
    char *label;
};
struct expressionsList {
    preparedExpression *expressions;
    int expressionsCount;
    ASTNode *astNode;
};
struct preparedCall {
    char *procedureName;
    expressionsList argumentExpressions;
    ASTNode *astNode;
};
struct preparedIndexer {
    preparedExpression *expression;
    expressionsList indexExpressions;
    ASTNode *astNode;
};

enum binaryType {
    BINARY_TYPE_PLUS,
    BINARY_TYPE_MINUS,
    BINARY_TYPE_STAR,
    BINARY_TYPE_SLASH,
    BINARY_TYPE_PERCENT,
    BINARY_TYPE_EQUAL,
    BINARY_TYPE_NOTEQUAL,
    BINARY_TYPE_LESSTHAN,
    BINARY_TYPE_GREATERTHAN,
    BINARY_TYPE_LESSTHANEQ,
    BINARY_TYPE_GREATERTHANEQ,
    BINARY_TYPE_AND,
    BINARY_TYPE_OR,
};
struct preparedBinary {
    binaryType type;
    preparedExpression *leftOperand;
    preparedExpression *rightOperand;
    ASTNode *astNode;
};
enum unaryType {
    UNARY_TYPE_MINUS,
    UNARY_TYPE_NOT
};
struct preparedUnary {
    unaryType type;
    preparedExpression *operand;
    ASTNode *astNode;
};
enum expressionType {
    UNARY,
    BINARY,
    BRACES,
    CALL,
    INDEXER,
    PLACE,
    LITERAL
};

struct preparedExpression {
    expressionType type;

    union {
        preparedUnary unary;
        preparedBinary binary;
        preparedExpression *expression;
        preparedCall call;
        preparedIndexer indexer;
        preparedLiteral literal;
        char *identifier;
    };
    ASTNode *astNode;
};
enum statementType {
    STATEMENT_TYPE_BLOCK,
    STATEMENT_TYPE_VAR,
    STATEMENT_TYPE_IF,
    STATEMENT_TYPE_WHILE,
    STATEMENT_TYPE_DO_WHILE,
    STATEMENT_TYPE_BREAK,
    STATEMENT_TYPE_EXPRESSION,
    STATEMENT_TYPE_ASSIGMENT,
    STATEMENT_TYPE_RETURN,
};
struct preparedBlock {
    preparedStatement *statements;
    int statementsCount;
    ASTNode *astNode;
};
struct preparedVars {
    preparedVar *vars;
    int count;
    ASTNode *astNode;
};
struct preparedWhile {
    preparedExpression condition;
    preparedBlock block;
    ASTNode *astNode;
};
struct preparedDoWhile {
    preparedExpression condition;
    preparedBlock block;
    ASTNode *astNode;
};
struct preparedAssigment {
    preparedExpression to;
    preparedExpression *rightPart;
    ASTNode *astNode;
};
struct preparedStatement {
    statementType type;
    union {
        preparedBlock block;
        preparedExpression expression;
        preparedIf *ifp;
        preparedVars vars;
        preparedWhile whilep;
        preparedDoWhile dowhile;
        preparedAssigment assigment;
    };
    ASTNode *astNode;
};


struct preparedFunc {
    char *identifier;
    preparedVars args;
    preparedType returnType;
    preparedBlock body;
    int seen;
    ASTNode *astNode;
};
struct conditionalStatement {
    preparedExpression condition;
    preparedStatement statement;
    ASTNode *astNode;
};
struct preparedIf {
    conditionalStatement statement;
    preparedStatement elseStatement;
    int elseStatementExists;
    ASTNode *astNode;
};


preparedBlock prepareBlock(ASTNode *body);

preparedExpression *prepareExpression(ASTNode *node);

preparedStatement prepareStatement(ASTNode *node);

preparedFunc prepareProcedure(ASTNode *procedure);

preparedFunc *prepareProcedures(ASTNodes allProcedures);

#endif //LAB_3_PREPROCESS_AST_H
