/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
*/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~ Joao Daniel Silva 86445 ~ Francisco do Canto Sousa 86416 ~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
 | * Function: escreveFicheiroAuxiliar
 | Description:	Funcao que cria um ficheiro auxiliar, escreve a matriz
 | para la e, no fim de escrever, substitui o original
 ---------------------------------------------------------------------*/

void escreveFicheiroAuxiliar(){
	FILE* apontoParaUmFile;
	apontoParaUmFile = fopen(tempFichS,"w");
	if (apontoParaUmFile == NULL) {
		fprintf(stderr, "\nErro: Falha a criar o ficheiro.\n");
		exit(-1);
	}
	
	dm2dPrintToFile(matrix_final, apontoParaUmFile);
	fclose(apontoParaUmFile);
	
	if (rename(tempFichS, fichS) != 0) {
		fprintf(stderr, "\nErro: Falha a renomear ficheiro.\n");
		exit(-1);
	}
}

void handlerSIGALRM(int num) {
	go_alarm = 1;
	alarm(periodo);
}

void handlerSIGINT(int num) {
	int estado;
	go_int = 0; alarm(0);
	if (!waitpid(-1, &estado, WNOHANG)) {
		wait(&estado);
	} else {
		escreveFicheiroAuxiliar();
	}
	exit(0);
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
		
		if (pthread_mutex_lock(&mutex) != 0) {
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
			if (go_alarm && !(go_alarm = 0) && waitpid(-1, &estado, WNOHANG)) {
				idproc = fork();
				
				if (idproc == -1) {
					fprintf(stderr, "\nErro: Nao foi possivel criar um processo filho.\n");
					exit(-1);
					
				} else if (idproc == 0) {
					/*Filho*/
					if (sigaction(SIGINT, &structIGN, NULL) != 0) {
						fprintf(stderr, "\nErro: Nao foi possivel atribuir a rotina de tratamento.\n");
						exit(-1);
					}
					escreveFicheiroAuxiliar();
					exit(0);
					
				} else {
					/*Pai*/
				}
			}
			
			if (pthread_cond_broadcast(&barreira) != 0) {
				fprintf(stderr, "\nErro: Falha a assinalar as condicoes.\n");
				exit(-1);
			}
			
		} else {
			while (i >= iteracao) {
				if (pthread_cond_wait(&barreira, &mutex) != 0) {
					fprintf(stderr, "\nErro: Falha a esperar pela condicao.\n");
					exit(-1);
				}
			}
		}
		if (pthread_mutex_unlock(&mutex) != 0) {
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
	if (N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || iteracoes < 1 || trab < 1 || N % trab != 0 || maxD < 0 || periodoS < 0 || strlen(fichS) == 0) {
		fprintf(stderr, "\nErro: Argumentos invalidos.\n");
		fprintf(stderr, "Uso: N >= 1, temperaturas >= 0,  trabalhadoras e iteracoes >= 1, diferenca maxima e periodo >= 0\n");
		return 1;
	}
	
	fprintf(stderr, "\nArgumentos:\n N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d trabalhadoras=%d maxD=%.2f ficheiro=%s periodo=%d\n", N, tEsq, tSup, tDir, tInf, iteracoes, trab, maxD, fichS, periodoS);
	
	/*Alocar memoria*/
	slave_args = (args_t*) malloc(trab*sizeof(args_t));
	slaves = (pthread_t*) malloc(trab*sizeof(pthread_t));
	tempFichS = (char*) malloc((strlen(fichS)+2)*sizeof(char));
	memset(&structSIGALRM, 0, sizeof(structSIGALRM));
	memset(&structSIGINT, 0, sizeof(structSIGINT));
	
	strcpy(tempFichS, fichS);
	strcat(tempFichS, "~");
	structSIGALRM.sa_handler = &handlerSIGALRM;
	structSIGINT.sa_handler = &handlerSIGINT;
	sigemptyset(&structSIGALRM.sa_mask);
	sigaddset(&structSIGALRM.sa_mask, SIGINT);
	sigemptyset(&structSIGINT.sa_mask);
	sigaddset(&structSIGINT.sa_mask, SIGALRM);
	
	/*Testa se existe o ficheiro*/
	filep = fopen(fichS,"r");
	if (filep != NULL) {
		matrix = readMatrix2dFromFile(filep, N+2, N+2);
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
	
	/*Rotinas de tratamento de interrupcoes*/
	if (sigaction(SIGALRM, &structSIGALRM, NULL) != 0) {
		fprintf(stderr, "\nErro: Nao foi possivel atribuir a rotina de tratamento.\n");
		return -1;
	}
	
	if (sigaction(SIGINT, &structSIGINT, NULL) != 0) {
		fprintf(stderr, "\nErro: Nao foi possivel atribuir a rotina de tratamento.\n");
		return -1;
	}
	
	/*Criar slaves*/
	klinhas = N/trab;
	periodo = periodoS;
	
	for (i = 0; i < trab; i++) {
		slave_args[i].id = i+1;
		slave_args[i].colunas = N;
		slave_args[i].klinhas = klinhas;
		slave_args[i].iter = iteracoes;
		slave_args[i].maxD = maxD;
		slave_args[i].matrix = matrix;
		slave_args[i].matrix_aux = matrix_aux;
		if (pthread_create(&slaves[i], NULL, slaveWork, &slave_args[i]) != 0) {
			fprintf(stderr, "\nErro: Um escravo falhou a criar.\n");
			return -1;
		}
	}
	
	alarm(periodo);
	
	/*Terminar threads*/
	for (i = 0; i < trab; i++) {
		if (pthread_join(slaves[i], NULL) != 0) {
			fprintf(stderr, "\nErro: Um escravo falhou a terminar.\n");
			return -1;
		}
	}
	/*Imprimir e apagar ficheiro*/
	dm2dPrint(matrix_final);
	
	if ((filep = fopen(fichS,"r")) && !fclose(filep) && unlink(fichS) != 0) {
		fprintf(stderr, "\nErro: Falhou a apagar o ficheiro.\n");
		return -1;
	}
	
	/*Libertar*/
	if (pthread_mutex_destroy(&mutex) != 0) {
		fprintf(stderr, "\nErro: Falhou a destruir mutex.\n");
		return -1;
	}
	
	if (pthread_cond_destroy(&barreira) != 0) {
		fprintf(stderr, "\nErro: Falhou a destruir variavel de condicao.\n");
		return -1;
	}
	
	dm2dFree(matrix);
	dm2dFree(matrix_aux);
	free(tempFichS);
	free(slave_args);
	free(slaves);
	
	return 0;
}
