#pragma once

#include "Standard.h"

static const char* TokenTypeStrings[] = {
  "STOP", "ERROR", "IDENTIFIER", "STRING", "CHARACTER",
  "NUMBER_B2", "NUMBER_B10", "NUMBER_B16", "NUMBER_FLOAT", "NUMBER_EXP",
  "PARENTHESIS_LEFT", "PARENTHESIS_RIGHT", "BRACKET_LEFT", "BRACKET_RIGHT",
  "BRACE_LEFT", "BRACE_RIGHT",
  "PLUS", "MINUS", "ASTERISK", "SLASH", "HASH", "AT", "TILDE", "UNDERSCORE", "CARET", "ASSIGN",
  "SHIFT_LEFT", "SHIFT_RIGHT", "ROTATE_LEFT", "ROTATE_RIGHT",
  "EXCLAMATION", "QUESTION", "COLON", "SEMICOLON", "COMMA", "DOT", "AMPERSAND", "BAR",
  "EQUAL", "NOT_EQUAL", "LESS", "LESS_EQUAL", "GREATER", "GREATER_EQUAL", "ARROW", "ELLIPSIS",
};

typedef enum {
  STOP, ERROR, IDENTIFIER, STRING, CHARACTER,
  NUMBER_B2, NUMBER_B10, NUMBER_B16, NUMBER_FLOAT, NUMBER_EXP,
  PARENTHESIS_LEFT, PARENTHESIS_RIGHT, BRACKET_LEFT, BRACKET_RIGHT,
  BRACE_LEFT, BRACE_RIGHT,
  PLUS, MINUS, ASTERISK, SLASH, HASH, AT, TILDE, UNDERSCORE, CARET, ASSIGN,
  SHIFT_LEFT, SHIFT_RIGHT, ROTATE_LEFT, ROTATE_RIGHT,
  EXCLAMATION, QUESTION, COLON, SEMICOLON, COMMA, DOT, AMPERSAND, BAR,
  EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL, ARROW, ELLIPSIS,
} tokentype;

typedef struct {
  char* Beginning;
  nat Length;
} stringview;

int CompareStringviews(stringview* Source1, stringview* Source2);
int CompareStringview(stringview* Source1, char* Source2);
nat StringviewToNat(stringview* Source, nat Base);

typedef struct {
  tokentype Type;
  stringview String;
  nat Line;
  nat Column;
} token;

typedef struct {
  char* SourceName;
  char* Cursor;
  nat Line;
  nat Column;
} tokenizer;

token TokenizeNext(tokenizer* Tokenizer);