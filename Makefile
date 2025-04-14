CC=gcc
CFLAGS =-Wall -Wextra -std=c99 -ggdb -pedantic
arq=processaAnotacoes.c

processaAnotacoes: $(arq) 
	$(CC) $(CFLAGS) -o processaAnotacoes $(arq) 

test: processaAnotacoes
	./processaAnotacoes in

testMem: processaAnotacoes
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./processaAnotacoes in 
