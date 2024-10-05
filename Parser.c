#include "Parser.h"

void InitializeNodeArray(nodearray* Array) {
  Array->Capacity = 2;
  Array->Cursor = 0;
  Array->Base = (astnode**)malloc(2 * sizeof(astnode*));
}

void AppendToNodeArray(nodearray* Array, astnode* Node) {
  if(Array->Cursor >= Array->Capacity) {
    Array->Capacity *= 2;
    Array->Base = (astnode**)realloc(Array->Base, Array->Capacity*sizeof(astnode*));
  }

  *(Array->Base + Array->Cursor) = Node;
  Array->Cursor++;
}

static void Advance(parser* Parser) {
  Parser->ExpectedToken = Parser->NextToken;
  Parser->NextToken = TokenizeNext(Parser->Tokenizer);
}

static nat Optional(parser* Parser, tokentype Type) {
  if(Parser->NextToken.Type == Type) {
    Advance(Parser);
    return 1;
  } else {
    return 0;
  }
}

static void Expect(parser* Parser, tokentype Type) {
  if(!Optional(Parser, Type)) {
    ERROR("%s%s:%d:%d:%s Expected %s got %s", RED, Parser->Tokenizer->SourceName,
    Parser->NextToken.Line, Parser->NextToken.Column, RESET, TokenTypeStrings[Type],
    TokenTypeStrings[Parser->NextToken.Type]);
  }
}

astnode* ParseExpression(parser* Parser);

astnode* ParseTerminal(parser* Parser) {
  astnode* Node = (astnode*)malloc(sizeof(astnode));

  if(Optional(Parser, NUMBER_B2)
  || Optional(Parser, NUMBER_B10)
  || Optional(Parser, NUMBER_B16)) {
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = B2NUMBER_NODE + Parser->ExpectedToken.Type - NUMBER_B2;
    Node->Number = Parser->ExpectedToken.String;
  } else if(Optional(Parser, STRING)) {
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = STRING_NODE;
    Node->String = Parser->ExpectedToken.String;
  } else if(Optional(Parser, CHARACTER)) {
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = CHARACTER_NODE;
    Node->Character = *Parser->ExpectedToken.String.Beginning;
  } else if(Optional(Parser, PARENTHESIS_LEFT)) {
    Node = ParseExpression(Parser);
    Expect(Parser, PARENTHESIS_RIGHT);
  } else {
    ERROR("%s:%d:%d: Expected terminal token got %s", Parser->Tokenizer->SourceName,
    Parser->NextToken.Line, Parser->NextToken.Column,
    TokenTypeStrings[Parser->NextToken.Type]);
  }

  return Node;
}

astnode* ParseIdentifier(parser* Parser) {
  if(Optional(Parser, GREATER)) {
    astnode* Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = POINTER_NODE;
    Node->Pointer = ParseIdentifier(Parser);

    return Node;
  } else if(Optional(Parser, IDENTIFIER)) {
    astnode* Node = (astnode*)malloc(sizeof(astnode));
    // handle procedure calls
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;

    if(Optional(Parser, PARENTHESIS_LEFT)) { 
      // input values
      if(Optional(Parser, IDENTIFIER)) {
        //Expect(Parser, IDENTIFIER);

        while(Optional(Parser, COMMA)) {
          //Expect(Parser, IDENTIFIER);
          Expect(Parser, IDENTIFIER);
        }
      }

      Expect(Parser, PARENTHESIS_RIGHT);

      if(Optional(Parser, ARROW)) {
        Expect(Parser, PARENTHESIS_LEFT);
        // output values
        if(Optional(Parser, IDENTIFIER)) {
          //Expect(Parser, IDENTIFIER);

          while(Optional(Parser, COMMA)) {
            //Expect(Parser, IDENTIFIER);
            Expect(Parser, IDENTIFIER);
          }
        }

        Expect(Parser, PARENTHESIS_RIGHT);
      }
    } else {
      Node->Type = IDENTIFIER_NODE;
      Node->Identifier.Base = Parser->ExpectedToken.String;
    }

    if(Optional(Parser, AT)) {
      Node->Identifier.Tail = ParseIdentifier(Parser);
    } else {
      Node->Identifier.Tail = NULL;
    }

    return Node;
  } else {
    return ParseTerminal(Parser);
  }
}

astnode* ParseUnary(parser* Parser) {
  if(Optional(Parser, MINUS)
  || Optional(Parser, TILDE)) {
    astnode* Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = UNARY_NODE;
    Node->Unary.Operator = Parser->ExpectedToken.Type == MINUS ? NEGATE_OP : INVERT_OP;
    Node->Unary.Operand = ParseUnary(Parser);

    return Node;
  } else if(Optional(Parser, LESS)) {
    astnode* Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = ADDRESS_NODE;
    Node->Address = ParseIdentifier(Parser); // ?

    return Node;
  } else {
    return ParseIdentifier(Parser);
  }
}

astnode* ParseShift(parser* Parser) {
  astnode* Node = ParseUnary(Parser);

  if(Optional(Parser, SHIFT_LEFT)
  || Optional(Parser, SHIFT_RIGHT)
  || Optional(Parser, ROTATE_LEFT)
  || Optional(Parser, ROTATE_RIGHT)) {
    astnode* Left = Node;
    Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = ARITHLOGIC_NODE;
    Node->ArithLogic.Operator = (arithlogicoperator)(Parser->ExpectedToken.Type-PLUS);
    Node->ArithLogic.Left = Left;
    Node->ArithLogic.Right = ParseShift(Parser);
  }

  return Node;
}

astnode* ParseLogical(parser* Parser) {
  astnode* Node = ParseShift(Parser);

  if(Optional(Parser, AMPERSAND)
  || Optional(Parser, BAR)
  || Optional(Parser, CARET)) {
    astnode* Left = Node;
    Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = ARITHLOGIC_NODE;
    Node->ArithLogic.Operator = (arithlogicoperator)(Parser->ExpectedToken.Type-PLUS);
    Node->ArithLogic.Left = Left;
    Node->ArithLogic.Right = ParseLogical(Parser);
  }

  return Node;
}

astnode* ParseFactor(parser* Parser) {
  astnode* Node = ParseLogical(Parser);

  if(Optional(Parser, ASTERISK)
  || Optional(Parser, SLASH)) {
    astnode* Left = Node;
    Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = ARITHLOGIC_NODE;
    Node->ArithLogic.Operator = (arithlogicoperator)(Parser->ExpectedToken.Type-PLUS);
    Node->ArithLogic.Left = Left;
    Node->ArithLogic.Right = ParseFactor(Parser);
  }

  return Node;
}

astnode* ParseTerm(parser* Parser) {
  astnode* Node = ParseFactor(Parser);

  if(Optional(Parser, PLUS)
  || Optional(Parser, MINUS)) {
    astnode* Left = Node;
    Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = ARITHLOGIC_NODE;
    Node->ArithLogic.Operator = (arithlogicoperator)(Parser->ExpectedToken.Type-PLUS);
    Node->ArithLogic.Left = Left;
    Node->ArithLogic.Right = ParseTerm(Parser);
  }

  return Node;
}

astnode* ParseAssignment(parser* Parser) {
  astnode* Node = ParseTerm(Parser);

  if(Optional(Parser, ASSIGN)) {
    astnode* Left = Node;
    Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = ASSIGNMENT_NODE;
    Node->Assignment.Left = Left;
    Node->Assignment.Right = ParseAssignment(Parser);
  }

  return Node;
}

astnode* ParseComparison(parser* Parser) {
  astnode* Node = ParseAssignment(Parser);

  if(Optional(Parser, EQUAL)
  || Optional(Parser, NOT_EQUAL)
  || Optional(Parser, LESS)
  || Optional(Parser, LESS_EQUAL)
  || Optional(Parser, GREATER)
  || Optional(Parser, GREATER_EQUAL)) {
    astnode* Left = Node;
    Node = (astnode*)malloc(sizeof(astnode));
    Node->Line = Parser->ExpectedToken.Line;
    Node->Column = Parser->ExpectedToken.Column;
    Node->Type = COMPARISON_NODE;
    Node->Comparison.Operator = (comparisonoperator)(Parser->ExpectedToken.Type-EQUAL);
    Node->Comparison.Left = Left;
    Node->Comparison.Right = ParseComparison(Parser);
  }
  
  return Node;
}

astnode* ParseExpression(parser* Parser) {
  return ParseComparison(Parser);
}

void DisplayAST(astnode* Node, nat Depth) {
  for(int i = 0; i < (int)Depth-1; i++) { printf("|   "); }
  if(Depth > 0) { printf("|-"); printf("> "); }

  switch(Node->Type) {
    case ROOT_NODE:
      printf("ROOT\n");
      for(nat i = 0; i < Node->Root.Cursor; i++) {
        DisplayAST(Node->Root.Base[i], Depth+1);
      }
    break;

    case COMPARISON_NODE: {
      char* COP[] = { "==", "!=", "<", "<=", ">", ">=" };
      printf("COMPARISON %s %d:%d\n", COP[Node->Comparison.Operator], Node->Line, Node->Column);
      DisplayAST(Node->Comparison.Left, Depth+1);
      DisplayAST(Node->Comparison.Right, Depth+1);
    } break;

    case ASSIGNMENT_NODE:
      printf("ASSIGN %d:%d\n", Node->Line, Node->Column);
      DisplayAST(Node->Assignment.Left, Depth+1);
      DisplayAST(Node->Assignment.Right, Depth+1);
    break;

    case ARITHLOGIC_NODE: {
      char* ALOP[] = { "+", "-", "*", "/", "&", "|", "^", "<<", ">>", "<<<", ">>>" };
      printf("ARTIH/LOGIC %s %d:%d\n", ALOP[Node->ArithLogic.Operator], Node->Line, Node->Column);
      DisplayAST(Node->ArithLogic.Left, Depth+1);
      DisplayAST(Node->ArithLogic.Right, Depth+1);
    } break;

    case UNARY_NODE:
      printf("%s %d:%d\n", Node->Unary.Operator == NEGATE_OP ? "NEGATE" : "INVERT", Node->Line, Node->Column);
      DisplayAST(Node->Unary.Operand, Depth+1);
    break;


    case POINTER_NODE:
      printf("VALUE OF ADDRESS %d:%d \n", Node->Line, Node->Column);
      DisplayAST(Node->Pointer, Depth+1);
    break;

    case ADDRESS_NODE:
      printf("ADDRESS OF/REFERENCE %d:%d\n", Node->Line, Node->Column);
      DisplayAST(Node->Address, Depth+1);
    break;

    case IDENTIFIER_NODE:
      printf("IDENTIFIER %.*s %d:%d\n", Node->Identifier.Base.Length, Node->Identifier.Base.Beginning, Node->Line, Node->Column);
      if(Node->Identifier.Tail != NULL) {
        DisplayAST(Node->Identifier.Tail, Depth+1);
      }
    break;

    case B2NUMBER_NODE:
    case B10NUMBER_NODE:
    case B16NUMBER_NODE:
      printf("NUMBER %.*s %d:%d\n", Node->Number.Length, Node->Number.Beginning, Node->Line, Node->Column);
    break;

    default:
      printf("UNKNOWN NODE TYPE %d %d:%d\n", (int)Node->Type, Node->Line, Node->Column);
    break;
  }
}

void Parse(parser* Parser) {
  // Construct AST
  Parser->RootNode.Type = ROOT_NODE;
  InitializeNodeArray(&Parser->RootNode.Root);

  Advance(Parser);

  while(Parser->NextToken.Type != STOP) {
    astnode* Node = ParseExpression(Parser);
    AppendToNodeArray(&Parser->RootNode.Root, Node);
  }

  DisplayAST(&Parser->RootNode, 0);
}