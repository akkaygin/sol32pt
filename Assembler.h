#pragma once

#include "Standard.h"
#include "Tokenizer.h"

typedef enum {
  BYTE_VT, DBYTE_VT, QBYTE_VT, ADDRESS_BFT,
  STRING_VT, BYTE_ARR_VT, DBYTE_ARR_VT, QBYTE_ARR_VT,
} variabletype;

typedef struct {
  stringview Identifier;
  nat Offset;
  nat Length;
  variabletype Type;
} variable;

typedef struct {
  nat Cursor;
  nat Capacity;
  variable* Variables;
} variabletable;

typedef struct {
  nat Cursor;
  nat Capacity;
  nat* Data;
} instructionarray;

typedef enum {
  LABEL_BFT, READ_BFT, WRITE_BFT
} backfilltype;

typedef struct {
  backfilltype Type;
  stringview Symbol;
  nat Offset;
} backfill;

typedef struct {
  nat Cursor;
  nat Capacity;
  backfill* Data;
} backfilltable;

typedef struct {
  tokenizer* Tokenizer;
  token ExpectedToken;
  token NextToken;

  nat Offset;
  backfilltable BackfillTable;
  variabletable VariableTable;
  instructionarray Instructions; // actually more than instructions
} assembler;

void Assemble(assembler* Assembler);
