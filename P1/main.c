/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~ Joao Daniel Silva 86445 ~ Francisco do Canto Sousa 86416 ~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


// Sistemas Operativos, DEI/IST/ULisboa 2017-18
// Projeto SO - exercicio 1, version 03

/*meter todos os aux para matrix aux, ver argumentos redundantes,
//gerir nomes de variaveis, meter mais funcoes*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "matrix2d.h"
#include "mplib3.h"

typedef struct {
	int id;
	int colunas;
	int klinhas;
	int iter;
} args_t;

/*--------------------------------------------------------------------
| Function: simulFatia
---------------------------------------------------------------------*/

DoubleMatrix2D* simulFatia(DoubleMatrix2D* matrix, DoubleMatrix2D* matrix_aux, int linhas, int colunas) {
	DoubleMatrix2D* act_matrix = matrix,* other = matrix_aux,* aux;
	double value;
	int l, c;

	if (linhas < 2 || colunas < 2) {
		return NULL;
	}

	for (l = 1; l < linhas-1; l++) {
		for (c = 1; c < colunas-1; c++) {
			value = (dm2dGetEntry(act_matrix, l-1, c) + dm2dGetEntry(act_matrix, l+1, c) + dm2dGetEntry(act_matrix, l, c-1) + dm2dGetEntry(act_matrix, l, c+1))/4.0;
			dm2dSetEntry(other, l, c, value);
		}
	}
	/*aux = other;
	other = act_matrix;
	act_matrix = aux;*/

	return other;
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

double parse_double_or_exit(char const *str, char const *name) {
	double value;
	if (sscanf(str, "%lf", &value) != 1 || value < 0) {
		fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
		exit(1);
	}
	return value;
}

/*--------------------------------------------------------------------
| Function: franciscoWork
--------------------------------------------------------------------*/

void* slaveWork(void* a){
	int i, iteracoes, myid, n, klinhas;
	DoubleMatrix2D* matrix,* matrix_aux,* matrix_res,* aux;
	args_t* args = (args_t*) a;
	double* rec_buffer;
	double* env_buffer;

	myid = args->id;
	n = args->colunas;
	klinhas = args->klinhas;
	iteracoes = args->iter;

	rec_buffer = (double*) malloc(sizeof(double)*(n+2));

	/*Inicializar matrix*/
	matrix = dm2dNew(klinhas+2, n+2);
	matrix_aux = dm2dNew(klinhas+2, n+2);

	/*Receber temperaturas iniciais*/
	receberMensagem(0, myid, rec_buffer, sizeof(double)*4);

	/*Inicializar matrix com argumentos*/
	dm2dSetColumnTo(matrix, 0, rec_buffer[0]);
	dm2dSetColumnTo(matrix, n+1, rec_buffer[2]);

	if (myid == 1) {
		dm2dSetLineTo(matrix, 0, rec_buffer[1]);
	} else if (myid == n/klinhas) {
		dm2dSetLineTo(matrix, klinhas+1, rec_buffer[3]);
	}

	/*Repetir na auxiliar*/
	dm2dCopy(matrix_aux, matrix);

	for (i = 0; i < iteracoes; i++) {
		matrix_res = simulFatia(matrix, matrix_aux, klinhas+2, n+2);
		if (myid%2) {
			if (myid != 1) { //Enviam linha de cima
				env_buffer = dm2dGetLine(matrix_aux, 1);
				enviarMensagem(myid, myid-1, env_buffer, sizeof(double)*(n+2));
				receberMensagem(myid-1, myid, rec_buffer, sizeof(double)*(n+2));
				dm2dSetLine(matrix_aux, 0, rec_buffer);
			}
			if (myid != n/klinhas) {
				env_buffer = dm2dGetLine(matrix_aux, klinhas);
				enviarMensagem(myid, myid+1, env_buffer, sizeof(double)*(n+2));
				receberMensagem(myid+1, myid, rec_buffer, sizeof(double)*(n+2));
				dm2dSetLine(matrix_aux, klinhas+1, rec_buffer);
			}
		}
		else {
			if (myid != 1) {
				env_buffer = dm2dGetLine(matrix_aux, 1);
				receberMensagem(myid-1, myid, rec_buffer, sizeof(double)*(n+2));
				enviarMensagem(myid, myid-1, env_buffer, sizeof(double)*(n+2));
				dm2dSetLine(matrix_aux, 0, rec_buffer);
			}
			if (myid != n/klinhas) {
				env_buffer = dm2dGetLine(matrix_aux, klinhas);
				receberMensagem(myid+1, myid, rec_buffer, sizeof(double)*(n+2));
				enviarMensagem(myid, myid+1, env_buffer, sizeof(double)*(n+2));
				dm2dSetLine(matrix_aux, klinhas+1, rec_buffer);
			}
		}
		matrix_aux = matrix;
		matrix = matrix_res;
	}
	/*Calcular valores*/
	/*result = simul(matrix, matrix_aux, N+2, N+2, iteracoes);
	//if (result == NULL) {
    	//printf("\nErro na simulacao.\n\n");
    	//return -1;
	}*/

	dm2dFree(matrix);
	dm2dFree(matrix_aux);
	free(rec_buffer);
	return 0;
}

/*--------------------------------------------------------------------
| Function: main
---------------------------------------------------------------------*/

int main(int argc, char** argv) {
	int i, nlinhas;
	/*DoubleMatrix2D *matrix, *matrix_aux, *result;*/
	double* buffer;
	args_t* slave_args;
	pthread_t* slaves;

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

	slave_args = (args_t*) malloc(trab*sizeof(args_t));
    slaves = (pthread_t*) malloc(trab*sizeof(pthread_t));

	buffer = (double*) malloc(sizeof(double)*(N+2));
	buffer[0] = tEsq; buffer[1] = tSup; buffer[2] = tDir; buffer[3] = tInf;

	/*Criar Threads*/
	if (inicializarMPlib(1, trab+1) == -1) {
		printf("Erro ao inicializar MPLib.\n");
		return 1;
	}

	/*Criar slaves*/
	nlinhas = N/trab;
	for (i=0; i<trab; i++) {
		slave_args[i].id = i+1;
	    slave_args[i].colunas = N;
		slave_args[i].klinhas = nlinhas;
		slave_args[i].iter = iteracoes;
	    pthread_create(&slaves[i], NULL, slaveWork, &slave_args[i]);
	}

	/*Enviar temperaturas iniciais*/
	for (i=1; i<trab+1; i++) {
		enviarMensagem(0, i, buffer, sizeof(double)*4);
	}


	for (i=1; i<trab+1; i++) {

	}


	/*Imprimir e terminar*/
	/*dm2dPrint(result);*/

	for (i = 0; i < trab; i++) {
		pthread_join(slaves[i], NULL);
  	}
	free(buffer);
	free(slave_args);
	free(slaves);
	libertarMPlib();
	return 0;
}
