//
// Created by trott on 19.01.2024.
//

#ifndef LAB_2_PAINTER_H
#define LAB_2_PAINTER_H

#include <stdio.h>
#include "../ast/ast.h"
#include "../ast/preprocess_ast.h"

ASTNodes findAllProcedures();


typedef struct CFG CFG;

CFG *makeCFG(preparedFunc pf, int nextId, int num);

void CFG_print(FILE *f, CFG *cfg, int num, CFG **pCfg, int i);

typedef struct Link Link;
typedef struct LinkList LinkList;
typedef struct Block Block;
typedef struct BlockList BlockList;
typedef struct CFGBuilder CFGBuilder;

// Block

struct Block {
    int id;
    __attribute__((unused)) char *call;
    LinkList *predecessors;
    LinkList *exits;
};

Block *NewBlock(int id, char *call);

// BlockList

struct BlockList {
    Block **blocks;
    int count;
};

BlockList *NewBlockList();


// CFGBuilder

struct CFGBuilder {
    BlockList *after_loop_block_stack;
    BlockList *curr_loop_guard_stack;
    Block *current_block;
    __attribute__((unused)) BlockList *calls;
    int current_id;
    CFG *cfg;
};

void CFGBuilder_visit(CFGBuilder *cfgBuilder, ASTNode *node);

// CFG

struct CFG {
    char *procedureName;
    Block *entryblock;
    __attribute__((unused)) BlockList *finalblocks;
    int nextId;
};

CFG *NewCFG(char *procedureName, Block *entryblock);

// Link

struct Link {
    Block *source;
    Block *target;
    char *comment;
};

Link *NewLink(Block *source, Block *target, char *string);

// LinkList

struct LinkList {
    Link **links;
    int count;
};

LinkList *NewLinkList();


#endif //LAB_2_PAINTER_H
