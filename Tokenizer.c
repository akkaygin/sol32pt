#include "Tokenizer.h"

#include <ctype.h>
#include <string.h>

int CompareStringviews(stringview* Source1, stringview* Source2) {
  nat ShortLength = Source1->Length;
  if(Source2->Length < ShortLength) {
    ShortLength = Source2->Length;
  }

  return strncmp(Source1->Beginning, Source2->Beginning, ShortLength);
}

int CompareStringview(stringview* Source1, char* Source2) {
  nat ShortLength = strlen(Source2);
  if(Source1->Length < ShortLength) {
    ShortLength = Source1->Length;
  }

  return strncmp(Source1->Beginning, Source2, ShortLength);
}

nat StringviewToNat(stringview* Source, nat Base) {
  nat Result = 0;

  if(Base == 0) {
    for(nat i = 2; i < Source->Length; i++) {
      if(Source->Beginning[i] != '_') {
        Result = Result << 1;
        Result = Result + Source->Beginning[i] - '0';
      }
    }
  } else if(Base == 1) {
    for(nat i = 0; i < Source->Length; i++) {
      if(Source->Beginning[i] != '_') {
        Result = Result * 10;
        Result = Result + Source->Beginning[i] - '0';
      }
    }
  } else if(Base == 2) {
    for(nat i = 2; i < Source->Length; i++) {
      if(Source->Beginning[i] != '_') {
        Result = Result << 4;
        if(Source->Beginning[i] <= '9') {
          Result = Result + Source->Beginning[i] - '0';
        } else if(Source->Beginning[i] <= 'F') {
          Result = Result + Source->Beginning[i] - 'A' + 10;
        } else {
          Result = Result + Source->Beginning[i] - 'a' + 10;
        }
      }
    }
  }

  return Result;
}

#define CURSOR *(Tokenizer->Cursor)
#define CURSOR_NEXT(Amount) *(Tokenizer->Cursor+Amount)

void AdvanceCursorOnce(tokenizer* Tokenizer) {
  if (CURSOR == '\n') {
    Tokenizer->Line++;
    Tokenizer->Column = 1;
  } else {
    Tokenizer->Column++;
  }

  Tokenizer->Cursor++;
}

void AdvanceCursor(tokenizer* Tokenizer, nat16 Length) {
  Tokenizer->Column += Length;
  Tokenizer->Cursor += Length;
  
  while(CURSOR == '\n') {
    Tokenizer->Column = 1;
    Tokenizer->Line += 1;
    Tokenizer->Cursor += 1;
  }
}

token CreateToken(tokentype Type, tokenizer* Tokenizer, nat16 Length) {
  token Token;
  Token.Type = Type;
  Token.String.Beginning = Tokenizer->Cursor;
  Token.String.Length = Length;
  Token.Line = Tokenizer->Line;
  Token.Column = Tokenizer->Column;

  AdvanceCursor(Tokenizer, Length);
  
  return Token;
}

token CreateIdentifierToken(tokenizer* Tokenizer, nat16 Length) {
  tokentype Type = IDENTIFIER;
  char* Begining = Tokenizer->Cursor;

  return CreateToken(IDENTIFIER, Tokenizer, Length);
}

token CreateStringToken(tokenizer* Tokenizer, nat16 Length) {
  tokentype Type = STRING;
  char* Begining = Tokenizer->Cursor;

  return CreateToken(Type, Tokenizer, Length);
}

token TokenizeNext(tokenizer* Tokenizer) {
  if(CURSOR == '/' && CURSOR_NEXT(1) == '/') {
    while(CURSOR != '\n' && CURSOR != '\0') {
      AdvanceCursorOnce(Tokenizer);
    }
  } else if(CURSOR == '/' && CURSOR_NEXT(1) == '*') {
    nat16 NestedCommentCount = 1;
    AdvanceCursorOnce(Tokenizer);
    AdvanceCursorOnce(Tokenizer);

    while(NestedCommentCount > 0 && CURSOR != '\0') {
      if(CURSOR == '/' && CURSOR_NEXT(1) == '*') {
        NestedCommentCount++;
        AdvanceCursorOnce(Tokenizer);
        AdvanceCursorOnce(Tokenizer);
      } else if(CURSOR == '*' && CURSOR_NEXT(1) == '/') {
        NestedCommentCount--;
        AdvanceCursorOnce(Tokenizer);
        AdvanceCursorOnce(Tokenizer);
      } else {
        AdvanceCursorOnce(Tokenizer);
      }
    }
  }
  
  while(isspace(CURSOR)) {
    AdvanceCursorOnce(Tokenizer);
  }

  token Result;
  if(isalpha(CURSOR) || CURSOR == '_') {
    if(CURSOR == '_' && !isalnum(CURSOR_NEXT(1))) {
      Result = CreateToken(UNDERSCORE, Tokenizer, 1);
    } else {
      nat16 Length = 1;
      while(isalnum(CURSOR_NEXT(Length)) || CURSOR_NEXT(Length) == '_') { Length++; }
      Result = CreateIdentifierToken(Tokenizer, Length);
    }
  } else if (isdigit(CURSOR)) {
    nat16 Length = 0;
    tokentype Type;
    if(CURSOR == '0' && CURSOR_NEXT(1) == 'x') {
      Length = 2;
      Type = NUMBER_B16;
      while(isxdigit(CURSOR_NEXT(Length)) || CURSOR_NEXT(Length) == '_') { Length++; }
    } else if(CURSOR == '0' && CURSOR_NEXT(1) == 'b') {
      Length = 2;
      Type = NUMBER_B2;
      while(CURSOR_NEXT(Length) == '1' || CURSOR_NEXT(Length) == '0' || CURSOR_NEXT(Length) == '_') { Length++; }
    } else {
      Type = NUMBER_B10;
      while(isdigit(CURSOR_NEXT(Length)) || CURSOR_NEXT(Length) == '_') {
        if(CURSOR_NEXT(Length+1) == '.') {
          if(Type == NUMBER_FLOAT) { ERROR("Invalid floating point number"); }
          Type = NUMBER_FLOAT;
          Length++;
        } else if(CURSOR_NEXT(Length+1) == 'e' || CURSOR_NEXT(Length+1) == 'E') {
          if(Type == NUMBER_EXP) { ERROR("Invalid ^10 number"); }
          Type = NUMBER_EXP;
          Length++;
        }
        Length++;
      }
    }
    Result = CreateToken(Type, Tokenizer, Length);
  } else {
    switch(CURSOR)
    {
      case '(':
        Result = CreateToken(PARENTHESIS_LEFT, Tokenizer, 1);
      break;

      case ')':
        Result = CreateToken(PARENTHESIS_RIGHT, Tokenizer, 1);
      break;

      case '[':
        Result = CreateToken(BRACKET_LEFT, Tokenizer, 1);
      break;

      case ']':
        Result = CreateToken(BRACKET_RIGHT, Tokenizer, 1);
      break;

      case '{':
        Result = CreateToken(BRACE_LEFT, Tokenizer, 1);
      break;

      case '}':
        Result = CreateToken(BRACE_RIGHT, Tokenizer, 1);
      break;

      case '+':
        Result = CreateToken(PLUS, Tokenizer, 1);
      break;

      case '-':
        if(CURSOR_NEXT(1) == '>') {
          Result = CreateToken(ARROW, Tokenizer, 2);
        } else {
          Result = CreateToken(MINUS, Tokenizer, 1);
        }
      break;

      case '*':
        Result = CreateToken(ASTERISK, Tokenizer, 1);
      break;

      case '/':
        Result = CreateToken(SLASH, Tokenizer, 1);
      break;

      case '#':
        Result = CreateToken(HASH, Tokenizer, 1);
      break;

      case '@':
        Result = CreateToken(AT, Tokenizer, 1);
      break;

      case '~':
        Result = CreateToken(TILDE, Tokenizer, 1);
      break;

      case '!':
        if(CURSOR_NEXT(1) == '=') {
          Result = CreateToken(NOT_EQUAL, Tokenizer, 2);
        } else {
          Result = CreateToken(EXCLAMATION, Tokenizer, 1);
        }
      break;

      case '?':
        Result = CreateToken(QUESTION, Tokenizer, 1);
      break;

      case ':':
        Result = CreateToken(COLON, Tokenizer, 1);
      break;

      case ';':
        Result = CreateToken(SEMICOLON, Tokenizer, 1);
      break;

      case ',':
        Result = CreateToken(COMMA, Tokenizer, 1);
      break;

      case '.':
        if(CURSOR_NEXT(1) == '.' && CURSOR_NEXT(2) == '.') {
          Result = CreateToken(ELLIPSIS, Tokenizer, 3); 
        } else {
          Result = CreateToken(DOT, Tokenizer, 1);
        }
      break;

      case '"': {
        nat32 Length = 1;
        while(CURSOR_NEXT(Length) != '"') {
          if(CURSOR_NEXT(Length) == '\0') {
            ERROR("%s:%d:%d: Unterminated string", Tokenizer->SourceName,
            Tokenizer->Line, Tokenizer->Column, CURSOR);
            Result = CreateToken(ERROR, Tokenizer, 0);
            break;  
          }
          Length++;
        }
        Length++;
        Result = CreateToken(STRING, Tokenizer, Length);
      }
      break;

      case '\'':
        if(CURSOR_NEXT(1) == '\\' && CURSOR_NEXT(2) != '\0' && CURSOR_NEXT(3) == '\'') {
          Result = CreateToken(CHARACTER, Tokenizer, 4);
        } else if(CURSOR_NEXT(1) != '\0' && CURSOR_NEXT(2) == '\'') {
          Result = CreateToken(CHARACTER, Tokenizer, 3);
        } else {
          ERROR("%s:%d:%d: Unterminated character", Tokenizer->SourceName,
          Tokenizer->Line, Tokenizer->Column);
          Result = CreateToken(ERROR, Tokenizer, 0);
        }
      break;

      case '&':
        Result = CreateToken(AMPERSAND, Tokenizer, 1);
      break;

      case '|':
        Result = CreateToken(BAR, Tokenizer, 1);
      break;

      case '^':
        Result = CreateToken(CARET, Tokenizer, 1);
      break;

      case '=':
        if(CURSOR_NEXT(1) == '=') {
          Result = CreateToken(EQUAL, Tokenizer, 2);
        } else {
          Result = CreateToken(ASSIGN, Tokenizer, 1);
        }
      break;

      case '>':
        if(CURSOR_NEXT(1) == '=') {
          Result = CreateToken(GREATER_EQUAL, Tokenizer, 2);
        } else if(CURSOR_NEXT(1) == '>') {
          if(CURSOR_NEXT(2) == '>') {
            Result = CreateToken(ROTATE_RIGHT, Tokenizer, 3);
          } else {
            Result = CreateToken(SHIFT_RIGHT, Tokenizer, 2);
          }
        } else {
          Result = CreateToken(GREATER, Tokenizer, 1);
        }
      break;

      case '<':
        if(CURSOR_NEXT(1) == '=') {
          Result = CreateToken(LESS_EQUAL, Tokenizer, 2);
        } else if(CURSOR_NEXT(1) == '<') {
          if(CURSOR_NEXT(2) == '<') {
            Result = CreateToken(ROTATE_LEFT, Tokenizer, 3);
          } else {
            Result = CreateToken(SHIFT_LEFT, Tokenizer, 2);
          }
        } else {
          Result = CreateToken(LESS, Tokenizer, 1);
        }
      break;

      case '\0':
        Result = CreateToken(STOP, Tokenizer, 0);
      break;

      default:
        ERROR("%s:%d:%d: Invalid token %c", Tokenizer->SourceName,
        Tokenizer->Line, Tokenizer->Column, CURSOR);
        Result = CreateToken(ERROR, Tokenizer, 0);
      break;
    }
  }

  return Result;
}