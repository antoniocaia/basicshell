main : util.o main.o executer.o parser.o lexer.o builtin.o 
	gcc -o notsosmart util.o main.o executer.o parser.o lexer.o builtin.o

util.o : headers.h util.c
	gcc -c util.c

main.o : headers.h main.c
	gcc -c main.c

execute.o : headers.h executer.c
	gcc -c executer.c

parser.o : headers.h parser.c
	gcc -c parser.c

lexer.o : headers.h lexer.c
	gcc -c lexer.c

builtin.o : headers.h builtin.c
	gcc -c builtin.c
