#include "Assembler.h"

/* Start of data structure related functions */
void InitializeBackfillTable(backfilltable* BackfillTable) {
  BackfillTable->Capacity = 2;
  BackfillTable->Cursor = 0;
  BackfillTable->Data = (backfill*)malloc(2 * sizeof(backfill));
}

void AppendToBackfillTable(assembler* Assembler, backfill* Backfill) {
  backfilltable* BackfillTable = &Assembler->BackfillTable;
  if(BackfillTable->Cursor >= BackfillTable->Capacity) {
    BackfillTable->Capacity *= 2;
    BackfillTable->Data = (backfill*)realloc(BackfillTable->Data,
      BackfillTable->Capacity*sizeof(backfill));
  }

  *(BackfillTable->Data + BackfillTable->Cursor) = *Backfill;
  BackfillTable->Cursor++;
}

void InitializeInstructionArray(instructionarray* InstructionArray) {
  InstructionArray->Capacity = 2;
  InstructionArray->Cursor = 0;
  InstructionArray->Data = (nat*)malloc(2 * sizeof(nat));
}

void AppendInstruction(assembler* Assembler, nat Instruction) {
  instructionarray* InstructionArray = &Assembler->Instructions;
  if(InstructionArray->Cursor >= InstructionArray->Capacity) {
    InstructionArray->Capacity *= 2;
    InstructionArray->Data = (nat*)realloc(InstructionArray->Data,
      InstructionArray->Capacity*sizeof(nat));
  }

  *(InstructionArray->Data + InstructionArray->Cursor) = Instruction;
  InstructionArray->Cursor++;
  Assembler->Offset++;
}

void InitializeVariableTable(variabletable* VariableTable) {
  VariableTable->Capacity = 2;
  VariableTable->Cursor = 0;
  VariableTable->Variables = (variable*)malloc(2 * sizeof(variable));
}

void AppendVariable(assembler* Assembler, variable Variable) {
  variabletable* VariableTable = &Assembler->VariableTable;
  if(VariableTable->Cursor >= VariableTable->Capacity) {
    VariableTable->Capacity *= 2;
    VariableTable->Variables = (variable*)realloc(VariableTable->Variables,
      VariableTable->Capacity*sizeof(variable));
  }

  *(VariableTable->Variables + VariableTable->Cursor) = Variable;
  VariableTable->Cursor++;
}

// make a hashmap
variable* SearchVariable(assembler* Assembler, stringview Target) {
  variabletable* VariableTable = &Assembler->VariableTable;
  for(int i = 0; i < VariableTable->Cursor; i++) {
    if(CompareStringviews(&Target, &((VariableTable->Variables + i)->Identifier)) == 0) {
      return (VariableTable->Variables + i);
    }
  }

  return NULL;
}

nat SwapEndianness(nat Data) {
  return ((Data & 0xFF) << 24) | ((Data & 0xFF00) << 8) | ((Data & 0xFF0000) >> 8) | ((Data & 0xFF000000) >> 24);
}
/* End of data structure related functions */

static void Advance(assembler* Assembler) {
  Assembler->ExpectedToken = Assembler->NextToken;
  Assembler->NextToken = TokenizeNext(Assembler->Tokenizer);
}

static nat Optional(assembler* Assembler, tokentype Type) {
  if(Assembler->NextToken.Type == Type) {
    Advance(Assembler);
    return 1;
  } else {
    return 0;
  }
}

static void Expect(assembler* Assembler, tokentype Type) {
  if(!Optional(Assembler, Type)) {
    ERROR("%s:%d:%d: Expected %s got %s", Assembler->Tokenizer->SourceName,
    Assembler->NextToken.Line, Assembler->NextToken.Column, TokenTypeStrings[Type],
    TokenTypeStrings[Assembler->NextToken.Type]);
  }
}

/*
  PROGRAM := (FILL | DATA | CONSTANT | INSTRUCTION)*
  FILL := NUMBER '*' WIDTH IDENTIFIER
  DATA := WIDTH IDENTIFIER ('=' NUMBER|CHAR)?
  CONSTANT := 'constant' IDENTIFIER STRING

  REG := 'r'NUMBER

  INSTRUCTION_ALU2_1 := INSTR_MNE('.' MOD)* REG REG REG ('+' NUMBER)?
  INSTRUCTION_ALU2_2 := ('MUL'|'DIV')('.' MOD)* REG REG REG

  INSTRUCTION_ALU1 := INSTR_MNE('.' MOD)* REG REG

  INSTRUCTION_MEM  := ('READ'|'WRITE')('.' MOD)* REG REG REG ('+' NUMBER)?

  INSTRUCTION_CSEL := 'CADD'('.' MOD)* 'r15' REG REG NUMBER

  INSTRUCTION_CCTR := 'CCTR'('.' MOD)* REG REG
*/

nat32 ParseE10S(assembler* Assembler) {
  nat Neg = Optional(Assembler, MINUS);
  
  int Number = 0;
  if(Optional(Assembler, NUMBER_B2)
  || Optional(Assembler, NUMBER_B10)
  || Optional(Assembler, NUMBER_B16)) {
    Number = StringviewToNat(&Assembler->ExpectedToken.String,
      Assembler->ExpectedToken.Type-NUMBER_B2);
  } else if(Optional(Assembler, CHARACTER)) {
    Number = (int)Assembler->ExpectedToken.String.Beginning[1];
  } else {
    ERROR("%s:%d:%d: Expected a constant got %.*s",
      Assembler->Tokenizer->SourceName, Assembler->NextToken.Line,
      Assembler->NextToken.Column, Assembler->NextToken.String.Length,
      Assembler->NextToken.String.Beginning);
  }
  
  if(Neg) {
    if(Number > 0x3FF+1) {
      WARN("Embedded value -%.*s in %s:%d:%d saturated to 10 bits",
        Assembler->ExpectedToken.String.Length,
        Assembler->ExpectedToken.String.Beginning,
        Assembler->Tokenizer->SourceName,
        Assembler->NextToken.Line, Assembler->NextToken.Column);
      Number = 0x3FF+1;
    }
    Number = Number * -1;
  } else {
    if(Number > 0x3FF) {
      WARN("Embedded value %.*s in %s:%d:%d truncated to 10 bits",
        Assembler->ExpectedToken.String.Length,
        Assembler->ExpectedToken.String.Beginning,
        Assembler->Tokenizer->SourceName,
        Assembler->NextToken.Line, Assembler->NextToken.Column);
      Number = Number & 0x3FF;
    }
  }
  
  return ((Number&0x3FF)<<1)|Neg;
}

nat32 ParseE12S(assembler* Assembler) {
  nat Neg = Optional(Assembler, MINUS);
  
  int Number = 0;
  if(Optional(Assembler, NUMBER_B2)
  || Optional(Assembler, NUMBER_B10)
  || Optional(Assembler, NUMBER_B16)) {
    Number = StringviewToNat(&Assembler->ExpectedToken.String,
      Assembler->ExpectedToken.Type-NUMBER_B2);
  } else if(Optional(Assembler, CHARACTER)) {
    Number = (int)Assembler->ExpectedToken.String.Beginning[1];
  } else {
    ERROR("%s:%d:%d: Expected a constant got %.*s",
      Assembler->Tokenizer->SourceName, Assembler->NextToken.Line,
      Assembler->NextToken.Column,
      Assembler->NextToken.String.Length, Assembler->NextToken.String.Beginning);
  }
  
  if(Neg) {
    if(Number > 0xFFF+1) {
      WARN("Embedded value -%.*s in %s:%d:%d saturated to 12 bits",
        Assembler->ExpectedToken.String.Length, Assembler->ExpectedToken.String.Beginning,
        Assembler->Tokenizer->SourceName, Assembler->NextToken.Line, Assembler->NextToken.Column);
      Number = 0xFF+1;
    }
    Number = Number * -1;
  } else {
    if(Number > 0xFFF) {
      WARN("Embedded value %.*s in %s:%d:%d truncated to 12 bits",
        Assembler->ExpectedToken.String.Length, Assembler->ExpectedToken.String.Beginning,
        Assembler->Tokenizer->SourceName, Assembler->NextToken.Line, Assembler->NextToken.Column);
      Number = Number & 0xFFF;
    }
  }
  
  return ((Number&0xFFF)<<1)|Neg;
}

nat32 ParseE16S(assembler* Assembler) {
  nat Neg = Optional(Assembler, MINUS);
  
  int Number = 0;
  if(Optional(Assembler, NUMBER_B2)
  || Optional(Assembler, NUMBER_B10)
  || Optional(Assembler, NUMBER_B16)) {
    Number = StringviewToNat(&Assembler->ExpectedToken.String,
      Assembler->ExpectedToken.Type-NUMBER_B2);
  } else if(Optional(Assembler, CHARACTER)) {
    Number = (int)Assembler->ExpectedToken.String.Beginning[1];
  } else {
    ERROR("%s:%d:%d: Expected a constant got %.*s",
      Assembler->Tokenizer->SourceName, Assembler->NextToken.Line,
      Assembler->NextToken.Column,
      Assembler->NextToken.String.Length, Assembler->NextToken.String.Beginning);
  }
  
  if(Neg) {
    if(Number > 0xFFFF+1) {
      WARN("Embedded value -%.*s in %s:%d:%d saturated to 16 bits",
        Assembler->ExpectedToken.String.Length, Assembler->ExpectedToken.String.Beginning,
        Assembler->Tokenizer->SourceName, Assembler->NextToken.Line, Assembler->NextToken.Column);
      Number = 0xFFFF+1;
    }
    Number = Number * -1;
  } else {
    if(Number > 0xFFFF) {
      WARN("Embedded value %.*s in %s:%d:%d truncated to 16 bits",
        Assembler->ExpectedToken.String.Length, Assembler->ExpectedToken.String.Beginning,
        Assembler->Tokenizer->SourceName, Assembler->NextToken.Line, Assembler->NextToken.Column);
      Number = Number & 0xFFFF;
    }
  }
  
  return ((Number&0xFFFF)<<1)|Neg;
}

char* ALU2Mnemonics[] = {
  "add", "and", "or", "____", "sl", "____", "____", "mul",
  "sub", "andi", "xor", "asr", "sr", "____", "____", "div",
};

char* ALU1Mnemonics[] = {
  "clz", "ctz", "csb", "____", "____", "____", "____", "abs",
  "orb", "revr", "revb", "____", "____", "____", "____", "par",
};

char* ComparisonOperators[] = {
  "eq", "ne", "uge", "ugt", "sge", "sgt", "____", "____",
  "zero", "neg", "crr", "ovf", "____", "____", "____", "____",
};

char* InvertedComparisonOperators[] = {
  "____", "____", "ule", "ult", "sle", "slt", "____", "____",
  "____", "____", "____", "____", "____", "____", "____", "____",
};

int OptionalRegister(assembler* Assembler) {
  // this may segfault on me someday
  if(*Assembler->ExpectedToken.String.Beginning == 'r') {
    stringview sv;
    char init[] = "0x ";
    init[2] = *(Assembler->ExpectedToken.String.Beginning + 1);
    sv.Beginning = init;
    sv.Length = 3;
    return StringviewToNat(&sv, 2);
  } else {
    return -1;
  }
}

int ExpectRegister(assembler* Assembler) {
  Expect(Assembler, IDENTIFIER);
  // lol
  if(*Assembler->ExpectedToken.String.Beginning == 'r') {
    stringview sv;
    char init[] = "0x ";
    init[2] = *(Assembler->ExpectedToken.String.Beginning + 1);
    sv.Beginning = init;
    sv.Length = 3;
    return StringviewToNat(&sv, 2);
  } else {
    ERROR("Expected register");
    return 0; // won't execute
  }
}

void AssembleALU2Instruction(assembler* Assembler, nat ALU2OP) {
  nat Result = 0;
  Result |= ALU2OP;

  if(ALU2OP == 7
  || ALU2OP == 15) {
    nat HLR = 0;
    nat SU = 0;
    while(Optional(Assembler, DOT)) {
      Expect(Assembler, IDENTIFIER);
      stringview* ModStr = &Assembler->ExpectedToken.String;
      if(CompareStringview(ModStr, "h") == 0) {
        HLR = 1;
      } else if(CompareStringview(ModStr, "r") == 0) {
        HLR = 1;
      } else if(CompareStringview(ModStr, "s") == 0) {
        SU = 1;
      } else {
        ERROR("%s:%d:%d: Expected one of MUL/DIV modifiers got %.*s",
        Assembler->Tokenizer->SourceName, Assembler->NextToken.Line,
        Assembler->NextToken.Column, ModStr->Length, ModStr->Beginning);
      }
    }
    Result |= HLR << 19;
    Result |= SU << 7;

    Result |= ExpectRegister(Assembler) << 28;
    Result |= ExpectRegister(Assembler) << 24;
    Result |= ExpectRegister(Assembler) << 20;
    
    AppendInstruction(Assembler, Result);
  } else {
    nat ShiftAmt = 0;
    if(Optional(Assembler, DOT)) {
      Expect(Assembler, IDENTIFIER);
      stringview* ModStr = &Assembler->ExpectedToken.String;
      if(CompareStringview(ModStr, "s8") == 0) {
        ShiftAmt = 1;
      } else if(CompareStringview(ModStr, "s16") == 0) {
        ShiftAmt = 2;
      } else if(CompareStringview(ModStr, "s24") == 0) {
        ShiftAmt = 3;
      } else {
        ERROR("%s:%d:%d: Expected one of ALU2 modifiers got %.*s",
        Assembler->Tokenizer->SourceName, Assembler->NextToken.Line,
        Assembler->NextToken.Column, ModStr->Length, ModStr->Beginning);
      }
    }

    Result |= ShiftAmt << 18;

    Result |= ExpectRegister(Assembler) << 28;
    Result |= ExpectRegister(Assembler) << 24;
    Result |= ExpectRegister(Assembler) << 20;
    
    nat32 E10S = 0;
    if(Optional(Assembler, PLUS)) {
      E10S = ParseE10S(Assembler);
    }
    
    Result |= E10S << 7;

    AppendInstruction(Assembler, Result);
  }
}

void AssembleALU1Instruction(assembler* Assembler, nat ALU1OP) {
  nat Result = 0;
  Result |= 2 << 4;
  Result |= ALU1OP;
  
  Result |= ExpectRegister(Assembler) << 28;
  Result |= ExpectRegister(Assembler) << 24;
  
  AppendInstruction(Assembler, Result);
}

void AssembleReadInstruction(assembler* Assembler) {
  nat32 Result = 0;
  Result |= 3 << 4;

  nat Signed = 0;
  nat Width = 0;
  while(Optional(Assembler, DOT)) {
    Expect(Assembler, IDENTIFIER);
    stringview* ModStr = &Assembler->ExpectedToken.String;
    if(CompareStringview(ModStr, "b") == 0) {
      Width = 0;
    } else if(CompareStringview(ModStr, "d") == 0) {
      Width = 1;
    } else if(CompareStringview(ModStr, "q") == 0) {
      Width = 2;
    } else if(CompareStringview(ModStr, "s") == 0) {
      Signed = 1;
    } else {
      ERROR("%s:%d:%d: Expected one of READ modifiers got %.*s",
      Assembler->Tokenizer->SourceName, Assembler->NextToken.Line,
      Assembler->NextToken.Column, ModStr->Length, ModStr->Beginning);
    }
  }
  Result |= Width;
  Result |= Signed << 2;

  Result |= ExpectRegister(Assembler) << 28;

  Expect(Assembler, IDENTIFIER);
  int OptReg = OptionalRegister(Assembler);
  if(OptReg != -1) {
    Result |= OptReg << 20;

    nat32 E12S = 0;
    if(Optional(Assembler, PLUS)) {
      E12S = ParseE12S(Assembler);
    }
    
    Result |= E12S << 7;
  } else {
    variable* OptVariable = SearchVariable(Assembler, Assembler->ExpectedToken.String);
    if(OptVariable == NULL) {
      backfill* NewBF = (backfill*)malloc(sizeof(backfill));
      NewBF->Type = READ_BFT;
      NewBF->Offset = Assembler->Offset;
      NewBF->Symbol = Assembler->ExpectedToken.String;
      AppendToBackfillTable(Assembler, NewBF);
    } else {
      int Offset = (OptVariable->Offset - Assembler->Offset) << 2;
      Result |= (Offset & 0xFFF) << 8;
      Result |= ((Offset >> 12) & 0xF) << 24;
      Result |= ((Offset < 0) ? 1 : 0) << 7;
    }
    Result |= 15 << 20;
  }

  AppendInstruction(Assembler, Result);
}

void AssembleWriteInstruction(assembler* Assembler) {
  nat32 Result = 0;
  Result |= 3 << 4;

  nat Signed = 0;
  nat Width = 0;
  while(Optional(Assembler, DOT)) {
    Expect(Assembler, IDENTIFIER);
    stringview* ModStr = &Assembler->ExpectedToken.String;
    if(CompareStringview(ModStr, "b") == 0) {
      Width = 0;
    } else if(CompareStringview(ModStr, "d") == 0) {
      Width = 1;
    } else if(CompareStringview(ModStr, "q") == 0) {
      Width = 2;
    } else {
      ERROR("%s:%d:%d: Expected one of WRITE modifiers got %.*s",
      Assembler->Tokenizer->SourceName, Assembler->NextToken.Line,
      Assembler->NextToken.Column, ModStr->Length, ModStr->Beginning);
    }
  }
  Result |= 0b1000 | Width;

  Result |= ExpectRegister(Assembler) << 24;

  Expect(Assembler, IDENTIFIER);
  int OptReg = OptionalRegister(Assembler);
  if(OptReg != -1) {
    Result |= OptReg << 20;

    nat32 E12S = 0;
    if(Optional(Assembler, PLUS)) {
      E12S = ParseE12S(Assembler);
    }
    
    Result |= E12S << 7;
  } else {
    variable* OptVariable = SearchVariable(Assembler, Assembler->ExpectedToken.String);
    if(OptVariable == NULL) {
      backfill* NewBF = (backfill*)malloc(sizeof(backfill));
      NewBF->Type = WRITE_BFT;
      NewBF->Offset = Assembler->Offset;
      NewBF->Symbol = Assembler->ExpectedToken.String;
      AppendToBackfillTable(Assembler, NewBF);
    } else {
      int Offset = (OptVariable->Offset - Assembler->Offset) << 2;
      Result |= (Offset & 0xFFF) << 8;
      Result |= ((Offset >> 12) & 0xF) << 28;
      Result |= ((Offset < 0) ? 1 : 0) << 7;
    }
    Result |= 15 << 20;
  }

  AppendInstruction(Assembler, Result);
}

void AssembleCJMPInstruction(assembler* Assembler) {
  nat32 Result = 0;
  Result |= 4 << 4;

  int CompareMode = -1;
  int Inverted = 0;
  if(Optional(Assembler, DOT)) {
    Expect(Assembler, IDENTIFIER);
    stringview ModStr = Assembler->ExpectedToken.String;
    for(nat i = 0; i < 16; i++) {
      if(CompareStringview(&ModStr, ComparisonOperators[i]) == 0) {
        CompareMode = i;
      } else if(CompareStringview(&ModStr, InvertedComparisonOperators[i]) == 0) {
        CompareMode = i;
        Inverted = 1;
      }
    }

    if(CompareMode == -1) {
      ERROR("%s:%d:%d: Expected one of comparison operators got %.*s",
      Assembler->Tokenizer->SourceName, Assembler->NextToken.Line,
      Assembler->NextToken.Column, ModStr.Length, ModStr.Beginning);
    }

    if(Inverted) {
      Result |= ExpectRegister(Assembler) << 20;
      Result |= ExpectRegister(Assembler) << 24;
    } else {
      Result |= ExpectRegister(Assembler) << 24;
      Result |= ExpectRegister(Assembler) << 20;
    }
  } else {
    CompareMode = 0;
  }

  Result |= CompareMode;

  if(Optional(Assembler, IDENTIFIER)) {
    variable* OptVariable = SearchVariable(Assembler, Assembler->ExpectedToken.String);
    if(OptVariable == NULL) {
      backfill* NewBF = (backfill*)malloc(sizeof(backfill));
      NewBF->Type = LABEL_BFT;
      NewBF->Offset = Assembler->Offset;
      NewBF->Symbol = Assembler->ExpectedToken.String;
      AppendToBackfillTable(Assembler, NewBF);
    } else {
      int Offset = (OptVariable->Offset - Assembler->Offset);
      Result |= (Offset & 0xFFF) << 8;
      Result |= (Offset >> 12) << 28;
      Result |= ((Offset < 0) ? 1 : 0) << 7;
    }
  } else {
    nat32 E16S = 0;
    E16S = ParseE16S(Assembler);
    
    Result |= (E16S & 0x1FFF) << 7;
    Result |= ((E16S >> 13) & 0xF) << 28;
  }
  
  AppendInstruction(Assembler, Result);
}

void AssembleCCTRInstruction(assembler* Assembler) {

}

void AssembleLoadAddress(assembler* Assembler) {
  nat Result = 0;

  Result |= ExpectRegister(Assembler) << 28;
  Result |= 15 << 20;

  Expect(Assembler, IDENTIFIER);

  variable* OptVariable = SearchVariable(Assembler, Assembler->ExpectedToken.String);
  if(OptVariable == NULL) {
    backfill* NewBF = (backfill*)malloc(sizeof(backfill));
    NewBF->Type = ADDRESS_BFT;
    NewBF->Offset = Assembler->Offset;
    NewBF->Symbol = Assembler->ExpectedToken.String;
    AppendToBackfillTable(Assembler, NewBF);
  } else {
    int Offset = (OptVariable->Offset - Assembler->Offset);
    Result |= (Offset & 0x3FF) << 8;
    Result |= ((Offset < 0) ? 1 : 0) << 7;
  }
}

variabletype StringviewToVariableType(stringview* Type) {
  if(CompareStringview(Type, "byte") == 0) {
    return BYTE_VT;
  } else if(CompareStringview(Type, "doublebyte") == 0) {
    return DBYTE_VT;
  } else if(CompareStringview(Type, "quadbyte") == 0) {
    return QBYTE_VT;
  } else if(CompareStringview(Type, "string") == 0) {
    return STRING_VT;
  } else {
    printf("v %.*s\n", Type->Length, Type->Beginning);
  }
}

void ParseVariable(assembler* Assembler, stringview* Type) {
  Expect(Assembler, IDENTIFIER);
  stringview Identifier = Assembler->ExpectedToken.String;
  
  variable Label;
  Label.Identifier = Identifier;
  Label.Type = StringviewToVariableType(Type);
  Label.Offset = Assembler->Offset;

  if(Label.Type == STRING_VT) {
    Expect(Assembler, STRING);
    stringview String = Assembler->ExpectedToken.String;

    nat Width = ((String.Length-2) & ~3) + ((String.Length-2) & 3 ? 4 : 0);
    char* Strmem = (char*)calloc(sizeof(char), Width);
    for(int i = 1; i < String.Length - 1; i++) {
      *(Strmem + i - 1) = *(String.Beginning + i);
    }
    Label.Length = Width;

    for(int i = 0; i < Width; i += 4) {
      nat Pack = *(Strmem + i + 3) | *(Strmem + i + 2) << 8 | *(Strmem + i + 1) << 16 | *(Strmem + i) << 24;
      AppendInstruction(Assembler, Pack);
    }
  } else {
    if(Optional(Assembler, NUMBER_B2)
    || Optional(Assembler, NUMBER_B10)
    || Optional(Assembler, NUMBER_B16)) {
      nat Data = StringviewToNat(&Assembler->ExpectedToken.String, Assembler->ExpectedToken.Type-NUMBER_B2);
      Data = SwapEndianness(Data);
      AppendInstruction(Assembler, Data);
    } else if(Optional(Assembler, BRACE_LEFT)) {
      // array?
      Expect(Assembler, BRACE_RIGHT);
    } else {
      ERROR("Expected constant value");
    }
  }

  AppendVariable(Assembler, Label);
}

void AssembleProgram(assembler* Assembler) {
  if(Optional(Assembler, IDENTIFIER)) {
    stringview String = Assembler->ExpectedToken.String;
    
    if(Optional(Assembler, COLON)) {
      variable Label;
      Label.Identifier = String;
      Label.Type = QBYTE_VT;
      Label.Offset = Assembler->Offset;
      AppendVariable(Assembler, Label);
      return;
    }
    
    for(nat i = 0; i < 16; i++) {
      if(CompareStringview(&String, ALU2Mnemonics[i]) == 0) {
        AssembleALU2Instruction(Assembler, i);
        return;
      }
    }
    
    for(nat i = 0; i < 8; i++) {
      if(CompareStringview(&String, ALU1Mnemonics[i]) == 0) {
        AssembleALU1Instruction(Assembler, i);
        return;
      }
    }

    if(CompareStringview(&String, "rd") == 0) {
      AssembleReadInstruction(Assembler);
    } else if(CompareStringview(&String, "wr") == 0) {
      AssembleWriteInstruction(Assembler);
    } else if(CompareStringview(&String, "j") == 0) {
      AssembleCJMPInstruction(Assembler);
    } else if(CompareStringview(&String, "cctr") == 0) {
      AssembleCCTRInstruction(Assembler);
    } else if(CompareStringview(&String, "la") == 0) {
      AssembleLoadAddress(Assembler);
    } else {
      ParseVariable(Assembler, &String);
    }
  }
}

void Assemble(assembler* Assembler) {
  Assembler->Offset = 0;
  InitializeBackfillTable(&Assembler->BackfillTable);
  InitializeVariableTable(&Assembler->VariableTable);
  InitializeInstructionArray(&Assembler->Instructions);

  Advance(Assembler);

  while(Assembler->NextToken.Type != STOP) {
    AssembleProgram(Assembler);
  }

  for(int i = 0; i < Assembler->BackfillTable.Cursor; i++) {
    backfill Backfill = *(Assembler->BackfillTable.Data + i);
    variable* OptVariable = SearchVariable(Assembler, Backfill.Symbol);
    if(OptVariable == NULL) {
      ERROR("Identifier %.*s (@0x%08X) never declared", Backfill.Symbol.Length, Backfill.Symbol.Beginning, Backfill.Offset);
    }

    int Offset = OptVariable->Offset - Backfill.Offset;
    if(Backfill.Type == LABEL_BFT) {
      *(Assembler->Instructions.Data + Backfill.Offset) |= (Offset & 0xFFF) << 8;
      *(Assembler->Instructions.Data + Backfill.Offset) |= ((Offset >> 12) & 0xF) << 28;
      *(Assembler->Instructions.Data + Backfill.Offset) |= ((Offset < 0) ? 1 : 0) << 7;
    } else if(Backfill.Type == WRITE_BFT) {
      //Offset = Offset << 1;
      *(Assembler->Instructions.Data + Backfill.Offset) |= (Offset & 0xFFF) << 8;
      *(Assembler->Instructions.Data + Backfill.Offset) |= ((Offset >> 12) & 0xF) << 28;
      *(Assembler->Instructions.Data + Backfill.Offset) |= ((Offset < 0) ? 1 : 0) << 7;
    } else if(Backfill.Type == READ_BFT) {
      //Offset = Offset << 1;
      *(Assembler->Instructions.Data + Backfill.Offset) |= (Offset & 0xFFF) << 8;
      *(Assembler->Instructions.Data + Backfill.Offset) |= ((Offset >> 12) & 0xF) << 24;
      *(Assembler->Instructions.Data + Backfill.Offset) |= ((Offset < 0) ? 1 : 0) << 7;
    } else if(Backfill.Type == ADDRESS_BFT) {
      *(Assembler->Instructions.Data + Backfill.Offset) |= (Offset & 0x3FF) << 8;
      *(Assembler->Instructions.Data + Backfill.Offset) |= ((Offset < 0) ? 1 : 0) << 7;
    }
  }

  for(int i = 0; i < Assembler->Instructions.Cursor; i++) {
    printf("0x%08X,\n", *(Assembler->Instructions.Data + i));
  }
}
