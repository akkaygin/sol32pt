#include "Standard.h"

#include "Tokenizer.h"
#include "Parser.h"
#include "Assembler.h"

typedef struct {
  nat Length;
  char* Data;
} sourcefile;

void LoadSource(sourcefile* SourceFile, char* Path) {
  FILE* FileHandle = fopen(Path, "r");
  if(FileHandle == NULL) {
    ERROR("fopen() failed");
  }
  
  fseek(FileHandle, 0, SEEK_END);
  SourceFile->Length = ftell(FileHandle);
  fseek(FileHandle, 0, SEEK_SET);

  SourceFile->Data = (char*)malloc(SourceFile->Length + 1);

  fread(SourceFile->Data, 1, SourceFile->Length, FileHandle);
  fclose(FileHandle);

  SourceFile->Data[SourceFile->Length] = '\0';
}

int main(int argc, char** argv) {
  if(argc < 2) {
    ERROR("No input file given");
  }

  sourcefile Source;
  LoadSource(&Source, argv[1]);
  DEBUG_INFO("Loaded %s", argv[1]);

  // preprocess?

  tokenizer Tokenizer;
  Tokenizer.Line   = 1;
  Tokenizer.Column = 1;
  Tokenizer.Cursor = Source.Data;
  Tokenizer.SourceName = argv[1];

  assembler Assembler;
  Assembler.Tokenizer = &Tokenizer;
  Assemble(&Assembler);
  return 0;

  parser Parser;
  Parser.Tokenizer = &Tokenizer;
  Parse(&Parser);

  return 0;
}