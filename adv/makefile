main : main.o execute.o parser.o builtin.o prompt.o
	gcc main.o execute.o parser.o builtin.o prompt.o

main.o : headers.h main.c
	gcc -c main.c

execute.o : headers.h execute.c
	gcc -c execute.c

parser.o : headers.h parser.c
	gcc -c parser.c

builtin.o : headers.h builtin.c
	gcc -c builtin.c

prompt.o : headers.h prompt.c
	gcc -c prompt.c
