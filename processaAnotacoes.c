#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define TAM_BUFFER 5000 
#define TAM_MAX_CONTEUDO 10000 
#define TAM_MAX_NOME 200
#define QTD_MAX_TITULOS 100 //Quantidade máxima de títulos principais

typedef struct Titulo{
	char * nome;
	char * conteudo;
	size_t profundidade;
	size_t tam_conteudo;
	size_t qtdSubtitulos;
	struct Titulo ** subtitulos;
 	struct Titulo * pai;
} Titulo;

int num_l = 1;

struct Titulo * ponteiros[100];
size_t qtdP = 0;

struct Titulo ** pparaP[100];
size_t qtdPPP = 0;

void liberaConteudo(Titulo * titulo){
	free(titulo->conteudo);	
	free(titulo->nome);	
	
	if(titulo->qtdSubtitulos > 0){
		pparaP[qtdPPP] = titulo->subtitulos;//Endereços de todos subtítulos que foram alocados
		qtdPPP++;

		for(size_t i = 0; i < titulo->qtdSubtitulos; ++i){
			liberaConteudo(titulo->subtitulos[i]);
			//printf("[pai %p]", titulo);
			//printf("%p\n", titulo->subtitulos[i]);
			ponteiros[qtdP] = titulo->subtitulos[i];//Endereços das structs Titulo, cujos ponteiros estão 
			//armazenados na var subtitulos do pai
			qtdP++;
		}
	}
}

void imprime(Titulo * titulo){
	printf("%s%s", titulo->nome, titulo->conteudo);
	
	if(titulo->qtdSubtitulos > 0){
		for(size_t i = 0; i < titulo->qtdSubtitulos; ++i)
			imprime(titulo->subtitulos[i]);
	}
}

void recebeConteudo(Titulo * titulo, char * linha){
	size_t tam_l = strlen(linha);
	size_t espaco_necessario = titulo->tam_conteudo + tam_l;
	
	if(espaco_necessario > TAM_MAX_CONTEUDO){
		errno = ENOMEM;
		perror("Limite máximo de conteúdo excedido");
		exit(1);
	}

	//Poderia dobrar de tamanho ao invés de realocar apenas o necessário, mas eu não quero
	titulo->conteudo = realloc(titulo->conteudo, espaco_necessario * sizeof(char));

	if(!titulo->conteudo){
		errno = ENOMEM;
		perror("Não foi possível alocar conteúdo por falta de memória");
		exit(1);
	}

	strcat(titulo->conteudo, linha);
	titulo->tam_conteudo = espaco_necessario; 
}

void inicializaTitulo(Titulo * titulo, char * nome){ //Só para testar como rascunho

	if((strlen(nome) + 1) > TAM_MAX_NOME){
		errno = ENOMEM;
		perror("Nome do título excede o limite máximo de caracteres");
		exit(1);
	}

	titulo->nome = malloc((strlen(nome) + 1) * sizeof(char));

	if(titulo->nome == NULL){
		errno = ENOMEM;
		perror("Não foi possível alocar nome do título por falta de memória");
		exit(1);
	}

	strcpy(titulo->nome, nome);	
	titulo->conteudo = malloc(sizeof(char));
	titulo->conteudo[0] = '\0';
	titulo->profundidade = 1;
	titulo->tam_conteudo = 1;
	titulo->qtdSubtitulos = 0;
	titulo->pai = NULL;
	titulo->subtitulos = NULL;
}

Titulo * criaSubtitulo(Titulo * tituloPai, char * linha){

	//Poderia dobrar de tamanho ao invés de realocar apenas o necessário, mas eu não quero
	tituloPai->subtitulos = realloc(tituloPai->subtitulos, (tituloPai->qtdSubtitulos + 1) * sizeof(Titulo *));	
	
	if(!tituloPai->subtitulos){
		errno = ENOMEM;
		perror("Não foi possível adicionar um subtítulo por falta de memória");
		exit(1);
	}

	Titulo * tituloCriado = (Titulo *)malloc(sizeof(Titulo));
	inicializaTitulo(tituloCriado, linha);
	tituloCriado->profundidade = tituloPai->profundidade + 1;
	tituloCriado->pai = tituloPai;

	tituloPai->subtitulos[tituloPai->qtdSubtitulos] = tituloCriado;
	
	tituloPai->qtdSubtitulos++;

	return tituloCriado;
}

int contaPrefixo(char * s){
	int c = 0, s_tam = strlen(s);

	assert(s_tam >= 1);

	for(int i = 0; i < s_tam; ++i){
		if(s[i] == '#')	
			++c;
		else
			break;
	}

	return c;
}

char * leArquivo(FILE * arq){
	char * buffer = malloc(TAM_BUFFER * sizeof (char));
	size_t tam_l;

	fgets(buffer, TAM_BUFFER, arq);
	
	if(feof(arq)){
		free(buffer);
		return NULL;
	}

	if(ferror(arq)){
		perror("Erro no arquivo");	
		exit(1);
	}

	tam_l = strlen(buffer);	

	if(!feof(arq) && buffer[tam_l-1] != '\n'){
		errno = ENOMEM;
		//printf("Linha %d tem mais de %d caracteres, último caracter: \"%c\"\n", num_l, TAM_BUFFER, buffer[tam_l - 1]);	
		perror("Há alguma linha no arquivo que é mais longa que o limite máximo");	
		exit(1);
	}
	
	num_l++;
	return buffer;
}

FILE * abreArquivo(const char * nomeArq){
	FILE * arq = fopen(nomeArq, "r+");
	if(arq == NULL){
		perror("Não foi possível abrir o arquivo");
		exit(1);
	}
	
	return arq;
}

int main(int argc, char **argv){
	char * linha;

	Titulo titulos[QTD_MAX_TITULOS];
	Titulo * tituloAtual = NULL;
	
	size_t qtdTitulos = 0, qtdPrefixos = 0;

	if(argc <= 1)
		return 1;

	FILE * arq = abreArquivo(argv[1]);

	for(linha = leArquivo(arq); linha != NULL; linha = leArquivo(arq)){
//		printf("<%d><%d#>%s", num_l, contaPrefixo(linha), linha);

		qtdPrefixos = contaPrefixo(linha);

		if(qtdPrefixos == 1){
			if((qtdTitulos + 1) >= QTD_MAX_TITULOS){
				errno = ENOMEM;
				perror("Quantidade máxima de títulos principais excedida.");
				exit(1);
			}

			inicializaTitulo(&titulos[qtdTitulos], linha);		
			tituloAtual = &titulos[qtdTitulos];
			++qtdTitulos;
		}
		else if(qtdPrefixos > tituloAtual->profundidade){
			tituloAtual = criaSubtitulo(tituloAtual, linha);
		}
		else if(qtdPrefixos == tituloAtual->profundidade){
			tituloAtual = criaSubtitulo(tituloAtual->pai, linha);
		}
		else if(qtdPrefixos > 0 && qtdPrefixos < tituloAtual->profundidade){
			do{
				tituloAtual = tituloAtual->pai;
			}while(qtdPrefixos <= tituloAtual->profundidade);

				tituloAtual = criaSubtitulo(tituloAtual, linha);
		}

		if(tituloAtual != NULL && qtdPrefixos == 0){
			recebeConteudo(tituloAtual, linha);
		}	

		free(linha);
	}

	for(size_t i = 0; i < qtdTitulos; ++i)
		imprime(&titulos[i]);

	for(size_t i = 0; i < qtdTitulos; ++i)
		liberaConteudo(&titulos[i]);//Libera espaço alocado para o conteudo

	for(size_t i = 0; i < qtdPPP; ++i)
		free(pparaP[i]);//Libera o espaço alocado para a array com os ponteiros dos subtítulos

	for(size_t i = 0; i < qtdP; ++i)
		free(ponteiros[i]);//Libera as structs dos subtítulos

	fclose(arq);
	return 0;
}


