#pragma once

#include "Standard.h"
#include "Tokenizer.h"

struct astnode;

typedef struct {
  nat Capacity;
  nat Cursor;
  struct astnode** Base;
} nodearray;

typedef enum {
  ROOT_NODE,

  COMPARISON_NODE,
  ASSIGNMENT_NODE,
  ARITHLOGIC_NODE,
  UNARY_NODE,
  
  POINTER_NODE, ADDRESS_NODE,
  IDENTIFIER_NODE,
  
  B2NUMBER_NODE, B10NUMBER_NODE, B16NUMBER_NODE,
  STRING_NODE,
  CHARACTER_NODE
} nodetype;

typedef enum {
  EQUAL_OP, NOT_EQUAL_OP,
  LESS_OP, LESS_EQUAL_OP,
  GREATER_OP, GREATER_EQUAL_OP
} comparisonoperator;

typedef struct {
  comparisonoperator Operator;
  struct astnode* Left;
  struct astnode* Right;
} comparison;

typedef enum {
  ADD_OP, SUB_OP, MUL_OP, DIV_OP,
  AND_OP, OR_OP, XOR_OP,
  SL_OP, SR_OP, RL_OP, RR_OP
} arithlogicoperator;

typedef struct {
  arithlogicoperator Operator;
  struct astnode* Left;
  struct astnode* Right;
} arithlogic;

typedef enum {
  NEGATE_OP, INVERT_OP
} unaryoperator;

typedef struct {
  unaryoperator Operator;
  struct astnode* Operand;
} unary;

typedef struct {
  struct astnode* Left;
  struct astnode* Right;
} assignment;

typedef struct identifier {
  stringview Base;
  struct astnode* Tail;
} identifier;

typedef struct astnode {
  nat Line;
  nat Column;

  nodetype Type;
  union {
    nodearray Root;

    comparison Comparison;
    assignment Assignment;
    arithlogic ArithLogic;
    unary Unary;

    struct astnode* Pointer;
    struct astnode* Address;
    identifier Identifier;

    stringview Number;
    stringview String;
    char Character;
  };
} astnode;

typedef struct {
  tokenizer* Tokenizer;
  astnode RootNode;
  token ExpectedToken;
  token NextToken;
} parser;

void InitializeNodeArray(nodearray* Array);
void AppendToNodeArray(nodearray* Array, astnode* Node);

void Parse(parser* Parser);