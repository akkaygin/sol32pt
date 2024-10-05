all:
	gcc Rocket.c Tokenizer.c Parser.c Assembler.c -o ./Build/rocket -g -DDEBUG

run: all
	./Build/rocket