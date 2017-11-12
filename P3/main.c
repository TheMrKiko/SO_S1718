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
#include <math.h>

#include "matrix2d.h"

#define omenor(A, B) A < B ? A : B
#define emenor(A, B) A < B ? 1 : 0
#define emaior(A, B) A > B ? 1 : 0

pthread_mutex_t mutex;
pthread_cond_t barreira;

/*--------------------------------------------------------------------
| Type: args_t
| Description: Estrutura com Informação para Trabalhadoras
---------------------------------------------------------------------*/

typedef struct {
	int id;
	int colunas;
	int klinhas;
	int iter;
	double maxD;
	DoubleMatrix2D* matrix;
	DoubleMatrix2D* matrix_aux;
	DoubleMatrix2D** pmatrix_res;
} args_t;

int threads_on_wait = 0, iteracao = 0, go_maxD = 1;
double max_min = 0;

void calcMaxMin(double th_min) {
	if (emaior(th_min, max_min)) {
		max_min = th_min;
	}
}

void atualizaGoMaxD(double maxD) {
	printf("ATUALIZA????????? para %f\n", max_min);
	if (emenor(max_min, maxD)) {
		go_maxD = 0;
		printf("omg atualizou\n" );
	}
	max_min = 0;
}

/*--------------------------------------------------------------------
| Function: simulFatia
| Description:	Função executada por cada tarefa trabalhadora a cada interacao.
				Processa um bloco da matriz dada como argumento.
---------------------------------------------------------------------*/

DoubleMatrix2D* simulFatia(DoubleMatrix2D* matrix, DoubleMatrix2D* matrix_aux, int linhas, int colunas, int linha_ini, double maxD, double* pmin) {
	int l, c;
	double value, diff;
	DoubleMatrix2D* act_matrix = matrix,* oth_matrix = matrix_aux;

	for (l = linha_ini; l < linha_ini + linhas; l++) {
		for (c = 1; c < colunas +1; c++) {
			value = (dm2dGetEntry(act_matrix, l-1, c) + dm2dGetEntry(act_matrix, l+1, c) + dm2dGetEntry(act_matrix, l, c-1) + dm2dGetEntry(act_matrix, l, c+1))/4.0;
			
			diff = fabs(value - dm2dGetEntry(act_matrix, l, c));
			*pmin = omenor(*pmin, diff);
			
			dm2dSetEntry(oth_matrix, l, c, value);
		}
	}
	return oth_matrix;
}

/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
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
| Function: parse_double_or_exit
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

/*--------------------------------------------------------------------
| Function: judaniWork
| Description: Função executada por cada tarefa trabalhadora.
|              Recebe como argumento uma estrutura do tipo args_t.
--------------------------------------------------------------------*/

void* slaveWork(void* a) {
	int i, iteracoes, myid, n, klinhas, linha_ini, trabs;
	double min_slave, maxD;
	DoubleMatrix2D* matrix,* matrix_aux,* matrix_res,** pmatrix_res;
	args_t* args = (args_t*) a;

	/*Ler argumentos*/
	myid = args->id;
	n = args->colunas;
	klinhas = args->klinhas;
	iteracoes = args->iter;
	maxD = args->maxD;
	matrix = args->matrix;
	matrix_aux = args->matrix_aux;
	pmatrix_res = args->pmatrix_res;

	linha_ini = (klinhas * (myid-1)) + 1;
	trabs = n/klinhas;
	min_slave = maxD * 2;

	/*Calcular valores*/
	for (i = 0; i < iteracoes && go_maxD; i++) {
		matrix_res = simulFatia(matrix, matrix_aux, klinhas, n, linha_ini, maxD, &min_slave);

		if (matrix_res == NULL) {
			fprintf(stderr, "\nErro na simulacao.\n");
			exit(-1);
		}
		matrix_aux = matrix;
		matrix = matrix_res;

		if (pthread_mutex_lock(&mutex)) {
			fprintf(stderr, "\nErro: Nao foi possivel obter o mutex.\n");
			exit(-1);
		}
		
		calcMaxMin(min_slave);
		threads_on_wait++;

		if (threads_on_wait == trabs) {
			threads_on_wait = 0;
			iteracao++;
			atualizaGoMaxD(maxD);

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
	if (myid == 1) *pmatrix_res = matrix_res;
	pthread_exit(NULL);
}

/*--------------------------------------------------------------------
| Function: main
| Description: Entrada do programa
---------------------------------------------------------------------*/

int main (int argc, char** argv) {
	int N, iteracoes, trab, klinhas, i;
	double tEsq, tSup, tDir, tInf, maxD;
	DoubleMatrix2D *matrix, *matrix_aux, *result;
	args_t* slave_args; pthread_t* slaves;

	if (argc != 9) {
		fprintf(stderr, "\nNumero invalido de argumentos.\n");
		fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf iteracoes trabalhadoras maxD\n\n");
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

	/*Validar argumentos*/
	if (N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || iteracoes < 1 || trab < 1 || N % trab != 0 || maxD < 0) {
		fprintf(stderr, "\nErro: Argumentos invalidos.\n");
		fprintf(stderr, "Uso: N >= 1, temperaturas >= 0,  trabalhadoras e iteracoes >= 1, diferenca maxima >= 0\n");
		return 1;
	}

	fprintf(stderr, "\nArgumentos:\n N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d trabalhadoras=%d maxD=%.2f\n", N, tEsq, tSup, tDir, tInf, iteracoes, trab, maxD);

	/*Criar matriz*/
	matrix = dm2dNew(N+2, N+2);
	matrix_aux = dm2dNew(N+2, N+2);

	if (matrix == NULL || matrix_aux == NULL) {
		fprintf(stderr, "\nErro: Nao foi possivel alocar memoria para as matrizes.\n\n");
		return -1;
	}

	/*Alocar slaves*/
	slave_args = (args_t*) malloc(trab*sizeof(args_t));
    slaves = (pthread_t*) malloc(trab*sizeof(pthread_t));

	/*Inicializar matriz*/
	dm2dSetLineTo(matrix, 0, tSup);
	dm2dSetLineTo(matrix, N+1, tInf);
	dm2dSetColumnTo(matrix, 0, tEsq);
	dm2dSetColumnTo(matrix, N+1, tDir);

	/*Repetir na auxiliar*/
	dm2dCopy(matrix_aux, matrix);

	/*Criar slaves*/
	klinhas = N/trab;
	for (i = 0; i < trab; i++) {
		slave_args[i].id = i+1;
	    slave_args[i].colunas = N;
		slave_args[i].klinhas = klinhas;
		slave_args[i].iter = iteracoes;
		slave_args[i].maxD = maxD;
		slave_args[i].matrix = matrix;
		slave_args[i].matrix_aux = matrix_aux;
		slave_args[i].pmatrix_res = &result;
	    if (pthread_create(&slaves[i], NULL, slaveWork, &slave_args[i])){
			fprintf(stderr, "\nErro: Um escravo falhou a criar.\n");
	        return -1;
		}
	}

	/*Terminar e Imprimir*/
	for (i = 0; i < trab; i++) {
		if (pthread_join(slaves[i], NULL)){
			fprintf(stderr, "\nErro: Um escravo falhou a terminar.\n");
	        return -1;
		}
	}

	dm2dPrint(result);

	dm2dFree(matrix);
	dm2dFree(matrix_aux);
	free(slave_args);
	free(slaves);
	return 0;
}
