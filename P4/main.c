/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
*/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~ Joao Daniel Silva 86445 ~ Francisco do Canto Sousa 86416 ~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "matrix2d.h"

#define omenor(A, B) A < B ? A : B
#define omaior(A, B) A > B ? A : B
#define emenor(A, B) A < B ? 1 : 0
#define emaior(A, B) A > B ? 1 : 0

pthread_mutex_t mutex;
pthread_cond_t barreira;

/*--------------------------------------------------------------------
| * Type: args_t
| Description: Estrutura com Informacao para Trabalhadoras
---------------------------------------------------------------------*/

typedef struct {
	int id;
	int colunas;
	int klinhas;
	int iter;
	double maxD;
	DoubleMatrix2D* matrix;
	DoubleMatrix2D* matrix_aux;
} args_t;

int threads_on_wait = 0, iteracao = 0, periodo = 0, go_maxD = 1, go_alarm = 0, go_int = 1;
double max_max = 0;
char* fichS,* tempFichS;
DoubleMatrix2D* matrix_final;

void calcMaxMax(double th_min) {
	if (emaior(th_min, max_max)) {
		max_max = th_min;
	}
}

void atualizaGoMaxD(double maxD) {
	if (emenor(max_max, maxD)) {
		go_maxD = 0;
	}
	max_max = 0;
}

void escreverFicheiroTemporario(){
	FILE* apontoParaUmFile;
	printf("SOU O %d A COMECAR A ESCREVER\n" , getpid() );
	apontoParaUmFile = fopen(tempFichS,"w");
	
	if (apontoParaUmFile == NULL) {
		perror(tempFichS);
	}
	
	dm2dPrintToFile(matrix_final, apontoParaUmFile);
	fclose(apontoParaUmFile);
	
	if (rename(tempFichS, fichS)) {
		fprintf(stderr, "\nErro: Falha a renomear ficheiro.\n");
		exit(-1);
	}
	printf("SOU O %d A ACABAR DE ESCREVER\n", getpid() );
}

void handlerSIGALRM(int num) {
	go_alarm = 1;
	alarm(periodo);
}

void handlerSIGINT(int num) {
	int estado;
	go_int = 0; alarm(0);
	if (!waitpid(-1, &estado, WNOHANG)) { /*se ja esta a haver uma escrita*/
		printf("SOU O %d e ALGUEM JA ESTA A MEIO\n", getpid());
		wait(&estado);
	} else {
		printf("SOU O %d e FAZENDO NOVA COPIA\n", getpid());
		
		escreverFicheiroTemporario();
	}
	exit(0);
}

/*--------------------------------------------------------------------
 | * Function: simulFatia
 | Description:	Funcao executada por cada tarefa trabalhadora a cada interacao.
 | Processa um bloco da matriz dada como argumento.
 ---------------------------------------------------------------------*/

DoubleMatrix2D* simulFatia(DoubleMatrix2D* matrix, DoubleMatrix2D* matrix_aux, int linhas, int colunas, int linha_ini, double maxD, double* pmax) {
	int l, c;
	double value, diff;
	DoubleMatrix2D* act_matrix = matrix,* oth_matrix = matrix_aux;
	
	for (l = linha_ini; l < linha_ini + linhas; l++) {
		for (c = 1; c < colunas +1; c++) {
			value = (dm2dGetEntry(act_matrix, l-1, c) + dm2dGetEntry(act_matrix, l+1, c) + dm2dGetEntry(act_matrix, l, c-1) + dm2dGetEntry(act_matrix, l, c+1))/4.0;
			
			diff = fabs(value - dm2dGetEntry(act_matrix, l, c));
			*pmax = omaior(*pmax, diff);
			
			dm2dSetEntry(oth_matrix, l, c, value);
		}
	}
	return oth_matrix;
}

/*--------------------------------------------------------------------
 | * Function: parse_integer_or_exit
 | Description: Processa a string str, do parâmetro name, como um inteiro
 ---------------------------------------------------------------------*/

int parse_integer_or_exit(char const *str, char const *name) {
	int value;
	
	if (sscanf(str, "%d", &value) != 1) {
		fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
		exit(1);
	}
	return value;
}

/*--------------------------------------------------------------------
 | * Function: parse_double_or_exit
 | Description: Processa a string str, do parâmetro name, como um double
 ---------------------------------------------------------------------*/

double parse_double_or_exit(char const *str, char const *name) {
	double value;
	
	if (sscanf(str, "%lf", &value) != 1) {
		fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
		exit(1);
	}
	return value;
}

/*-------------------------------------------------------------------
| * Function: judaniWork
| Description: Funcao executada por cada tarefa trabalhadora.
|              Recebe como argumento uma estrutura do tipo args_t.
--------------------------------------------------------------------*/

void* slaveWork(void* a) {
	int i, iteracoes, myid, n, klinhas, linha_ini, trabs, estado, idproc;
	double max_slave, maxD;
	DoubleMatrix2D* matrix,* matrix_aux,* matrix_res;
	args_t* args = (args_t*) a;
	struct sigaction structIGN;
	
	/*Ler argumentos*/
	myid = args->id;
	n = args->colunas;
	klinhas = args->klinhas;
	iteracoes = args->iter;
	maxD = args->maxD;
	matrix = args->matrix;
	matrix_aux = args->matrix_aux;
	
	linha_ini = (klinhas * (myid-1)) + 1;
	trabs = n/klinhas;
	max_slave = 0;
	
	memset(&structIGN, 0, sizeof(structIGN));
	structIGN.sa_handler = SIG_IGN;
	
	for (i = 0; i < iteracoes && go_maxD && go_int; i++) {
		/*Calcular matriz*/
		matrix_res = simulFatia(matrix, matrix_aux, klinhas, n, linha_ini, maxD, &max_slave);
		
		if (matrix_res == NULL) {
			fprintf(stderr, "\nErro: Falha na simulacao.\n");
			exit(-1);
		}
		matrix_aux = matrix;
		matrix = matrix_res;
		
		if (pthread_mutex_lock(&mutex)) {
			fprintf(stderr, "\nErro: Nao foi possivel obter o mutex.\n");
			exit(-1);
		}
		
		threads_on_wait++;
		calcMaxMax(max_slave);
		max_slave = 0;
		
		if (threads_on_wait == trabs) {
			threads_on_wait = 0;
			iteracao++;
			atualizaGoMaxD(maxD);
			
			matrix_final = matrix_res;
			if (go_alarm) printf("ALARME %ld\n", pthread_self());
			
			if (go_alarm && !(go_alarm = 0) && waitpid(-1, &estado, WNOHANG)) {
				
				idproc = fork();
				
				if (idproc == -1) {
					printf("Erro: Nao foi possivel criar um processo filho.\n");
					
				} else if (idproc == 0) {
					/*Filho*/
					
					if (sigaction(SIGINT, &structIGN, NULL) != 0) {
						printf("Erro: \n");
					}
					
					escreverFicheiroTemporario();
					exit(0);
				} else {
					/*Pai*/
					
				}
			}
			
			if (pthread_cond_broadcast(&barreira)) {
				fprintf(stderr, "\nErro: Falha a assinalar as condicoes.\n");
				exit(-1);
			}
			
		} else {
			while (i >= iteracao) {
				if (pthread_cond_wait(&barreira, &mutex)) {
					fprintf(stderr, "\nErro: Falha a esperar pela condicao.\n");
					exit(-1);
				}
			}
		}
		if (pthread_mutex_unlock(&mutex)) {
			fprintf(stderr, "\nErro: Nao foi possivel libertar o mutex.\n");
			exit(-1);
		}
		
	}
	return NULL;
}

/*--------------------------------------------------------------------
| * Function: main
| Description: Entrada do programa
---------------------------------------------------------------------*/

int main (int argc, char** argv) {
	int N, iteracoes, trab, klinhas, periodoS, i;
	double tEsq, tSup, tDir, tInf, maxD;
	DoubleMatrix2D *matrix, *matrix_aux;
	args_t* slave_args; pthread_t* slaves;
	struct sigaction structSIGALRM, structSIGINT;
	FILE* filep;
	printf("SOU O PAI %d\n" , getpid() );
	
	if (argc != 11) {
		fprintf(stderr, "\nNumero invalido de argumentos.\n");
		fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf iteracoes trabalhadoras maxD ficheiro periodo\n\n");
		return 1;
	}
	
	/* argv[0] = program name */
	N = parse_integer_or_exit(argv[1], "N");
	tEsq = parse_double_or_exit(argv[2], "tEsq");
	tSup = parse_double_or_exit(argv[3], "tSup");
	tDir = parse_double_or_exit(argv[4], "tDir");
	tInf = parse_double_or_exit(argv[5], "tInf");
	iteracoes = parse_integer_or_exit(argv[6], "iteracoes");
	trab = parse_integer_or_exit(argv[7], "trabalhadoras");
	maxD = parse_double_or_exit(argv[8], "diferenca_maxima");
	fichS = argv[9];
	periodoS = parse_integer_or_exit(argv[10], "periodo");
	
	/*Validar argumentos*/
	if (N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || iteracoes < 1 || trab < 1 || N % trab != 0 || maxD < 0 || periodoS < 0 || !strlen(fichS)) {
		fprintf(stderr, "\nErro: Argumentos invalidos.\n");
		fprintf(stderr, "Uso: N >= 1, temperaturas >= 0,  trabalhadoras e iteracoes >= 1, diferenca maxima e periodo >= 0\n");
		return 1;
	}
	
	fprintf(stderr, "\nArgumentos:\n N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d trabalhadoras=%d maxD=%.2f ficheiro=%s periodo=%d\n", N, tEsq, tSup, tDir, tInf, iteracoes, trab, maxD, fichS, periodoS);
	
	/*Alocar slaves*/
	slave_args = (args_t*) malloc(trab*sizeof(args_t));
	slaves = (pthread_t*) malloc(trab*sizeof(pthread_t));
	tempFichS = (char*) malloc((strlen(fichS)+1)*sizeof(char));
	memset(&structSIGALRM, 0, sizeof(structSIGALRM));
	strcpy(tempFichS, fichS);
	strcat(tempFichS, "~");
	structSIGALRM.sa_handler = &handlerSIGALRM;

	
	/*Testa se existe o ficheiro*/
	filep = fopen(fichS,"r");
	if (filep) {
		matrix = readMatrix2dFromFile(filep, N+2, N+2);
		//dm2dPrint(matrix);
		printf("printou\n");
		fclose(filep);
	} else {
		matrix = dm2dNew(N+2, N+2);
	}
	
	/*Criar matriz aux*/
	matrix_aux = dm2dNew(N+2, N+2);
	
	if (matrix == NULL || matrix_aux == NULL) {
		fprintf(stderr, "\nErro: Nao foi possivel alocar memoria para as matrizes.\n\n");
		return -1;
	}
	
	/*Inicializar matriz*/
	dm2dSetLineTo(matrix, 0, tSup);
	dm2dSetLineTo(matrix, N+1, tInf);
	dm2dSetColumnTo(matrix, 0, tEsq);
	dm2dSetColumnTo(matrix, N+1, tDir);
	
	/*Repetir na auxiliar*/
	dm2dCopy(matrix_aux, matrix);
	
	if (sigaction(SIGALRM, &structSIGALRM, NULL) != 0) {
		printf("Erro: \n");
	}
	
	structSIGINT.sa_handler = &handlerSIGINT;
	if (sigaction(SIGINT, &structSIGINT, NULL) != 0) {
		printf("Erro: \n");
	}
	/*Criar slaves*/
	klinhas = N/trab;
	periodo = periodoS;
	alarm(periodo);
	
	for (i = 0; i < trab; i++) {
		slave_args[i].id = i+1;
		slave_args[i].colunas = N;
		slave_args[i].klinhas = klinhas;
		slave_args[i].iter = iteracoes;
		slave_args[i].maxD = maxD;
		slave_args[i].matrix = matrix;
		slave_args[i].matrix_aux = matrix_aux;
		if (pthread_create(&slaves[i], NULL, slaveWork, &slave_args[i])) {
			fprintf(stderr, "\nErro: Um escravo falhou a criar.\n");
			return -1;
		}
	}
	printf("MAIN %ld\n", pthread_self());
	
	/*Terminar threads*/
	for (i = 0; i < trab; i++) {
		if (pthread_join(slaves[i], NULL)) {
			fprintf(stderr, "\nErro: Um escravo falhou a terminar.\n");
			return -1;
		}
	}
	/*Apagar ficheiro*/
	if ((filep = fopen(fichS,"r")) && !fclose(filep) && unlink(fichS) != 0) {
		fprintf(stderr, "\nErro: Falhou a apagar o ficheiro.\n");
		return -1;
	}
	
	/*Libertar e Imprimir*/
	if (pthread_mutex_destroy(&mutex) != 0) {
		fprintf(stderr, "\nErro: Falhou a destruir mutex.\n");
		return -1;
	}
	
	if (pthread_cond_destroy(&barreira) != 0) {
		fprintf(stderr, "\nErro: Falhou a destruir variavel de condicao.\n");
		return -1;
	}
	
	dm2dPrint(matrix_final);
	printf("printou\n");
	
	dm2dFree(matrix);
	dm2dFree(matrix_aux);
	free(tempFichS);
	free(slave_args);
	free(slaves);
	return 0;
}
