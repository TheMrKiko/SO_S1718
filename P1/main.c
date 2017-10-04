/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "matrix2d.h"

typedef struct args {
	int id;
	int colunas;
	int klinhas;
	DoubleMatrix2D matrix_t;
	DoubleMatrix2D aux_t;
} Args_t;
/*--------------------------------------------------------------------
| Function: simul
---------------------------------------------------------------------*/

DoubleMatrix2D* simul(DoubleMatrix2D* matrix, DoubleMatrix2D* matrix_aux, int linhas, int colunas, int numIteracoes) {
	DoubleMatrix2D *act_matrix = matrix, *other = matrix_aux, *aux;
	double value;
	int l, c, i;

	if (linhas < 2 || colunas < 2) {
		return NULL;
	}

	for (i = 0; i < numIteracoes; i++) {

		for (l = 1; l < linhas-1; l++) {
			for (c = 1; c < colunas-1; c++) {
				value = (dm2dGetEntry(act_matrix, l-1, c) + dm2dGetEntry(act_matrix, l+1, c) + dm2dGetEntry(act_matrix, l, c-1) + dm2dGetEntry(act_matrix, l, c+1))/4.0;
				dm2dSetEntry(other, l, c, value);
				/*dm2dPrint(other);*/
			}

		}
		aux = other;
		other = act_matrix;
		act_matrix = aux;
	}
	return act_matrix;
}

/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
---------------------------------------------------------------------*/

int parse_integer_or_exit(char const *str, char const *name)
{
	int value;

	if (sscanf(str, "%d", &value) != 1 || value < 1) {
		fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
		exit(1);
	}
	return value;
}

/*--------------------------------------------------------------------
| Function: parse_double_or_exit
---------------------------------------------------------------------*/

double parse_double_or_exit(char const *str, char const *name)
{
	double value;

	if (sscanf(str, "%lf", &value) != 1 || value < 0) {
		fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
		exit(1);
	}
	return value;
}

void* slavework(void* a){
	DoubleMatrix2D *matrix_t;
	DoubleMatrix2D *aux_t;
}


/*--------------------------------------------------------------------
| Function: main
---------------------------------------------------------------------*/

int main (int argc, char** argv) {
	int i, nlinhas;
	/*DoubleMatrix2D *matrix, *matrix_aux, *result;*/

	Args_t *slave_args;
	pthread_t *slaves;

	if (argc != 9) {
		fprintf(stderr, "\nNumero invalido de argumentos.\n");
		fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf iteracoes trabalhadoras mensagens_por_canal\n\n");
		return 1;
	}

	/* argv[0] = program name */
	int N = parse_integer_or_exit(argv[1], "N");
	double tEsq = parse_double_or_exit(argv[2], "tEsq");
	double tSup = parse_double_or_exit(argv[3], "tSup");
	double tDir = parse_double_or_exit(argv[4], "tDir");
	double tInf = parse_double_or_exit(argv[5], "tInf");
	int iteracoes = parse_integer_or_exit(argv[6], "iteracoes");
	int trab = parse_integer_or_exit(argv[7], "trabalhadoras");
	int csz = parse_integer_or_exit(argv[8], "mensagens_por_canal");


	/*Verificar argumentos*/
	if (N<1 || tEsq<0 || tSup<0 || tDir<0 || tInf<0 || iteracoes<1 || trab<1 || N%trab != 0 || csz<0) {
		exit(1);
	}

	fprintf(stderr, "\nArgumentos:\nN=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d trabalhadoras=%d mensagens_por_canal=%d\n", N, tEsq, tSup, tDir, tInf, iteracoes, trab, csz);

	/*Inicializar matriz*/
	matrix = dm2dNew(N+2, N+2);
	matrix_aux = dm2dNew(N+2, N+2);

	/*Inicializar matriz com argumentos*/
	dm2dSetLineTo(matrix, 0, tSup);
	dm2dSetLineTo(matrix, N+1, tInf);
	dm2dSetColumnTo(matrix, 0, tEsq);
	dm2dSetColumnTo(matrix, N+1, tDir);

	/*Repetir na auxiliar*/
	dm2dCopy(matrix_aux, matrix);


	slave_args = (Args_t*) malloc(trab*sizeof(Args_t));
    slaves = (pthread_t*) malloc(trab*sizeof(pthread_t));

	/*Criar Threads*/
	if (inicializarMPlib(1, trab+1) == -1) {
		printf("Erro ao inicializar MPLib.\n");
		return 1;
	}

	/* create slaves*/
	nlinhas = N/trab;
	for (i=0; i<trab; i++) {
		slave_args[i].id = i+1;
	    slave_args[i].colunas = N;
		slave_args[i].klinhas = nlinhas;
		slave_args[i].matrix_t = dm2dNew(nlinhas+2, N+2);
		slave_args[i].aux_t = dm2dNew(nlinhas+2, N+2);
	    pthread_create(&slaves[i], NULL, slaveThread, &slave_args[i]);
	}

	/*
	thread0->
	thread_create(t1, nulll, slaveworks, (void*)args)*/

	/*Calcular valores*/
	result = simul(matrix, matrix_aux, N+2, N+2, iteracoes);
	if (result == NULL) {
    	printf("\nErro na simulacao.\n\n");
    	return -1;
	}

	/*Imprimir e terminar*/
	dm2dPrint(result);
	dm2dFree(matrix);
	dm2dFree(matrix_aux);

	return 0;
}
