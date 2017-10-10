/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "matrix2d.h"


/*--------------------------------------------------------------------
| Function: simul
---------------------------------------------------------------------*/

DoubleMatrix2D* simul(DoubleMatrix2D* matrix, DoubleMatrix2D* matrix_aux, int linhas, int colunas, int numIteracoes) {
	DoubleMatrix2D *act_matrix = matrix, *other = matrix_aux, *aux;
	double value;
	int l, c, i;

	for (i = 0; i < numIteracoes; i++) {

		for (l = 1; l < linhas-1; l++) {
			for (c = 1; c < colunas-1; c++) {
				value = (dm2dGetEntry(act_matrix, l-1, c) + dm2dGetEntry(act_matrix, l+1, c) + dm2dGetEntry(act_matrix, l, c-1) + dm2dGetEntry(act_matrix, l, c+1))/4.0;
				dm2dSetEntry(other, l, c, value);
				
			};

		};
        dm2dPrint(other);
		aux = act_matrix;
		act_matrix = other;
		other = aux;
	};
	return act_matrix;
}

/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
---------------------------------------------------------------------*/

int parse_integer_or_exit(char const *str, char const *name)
{
	int value;

	if(sscanf(str, "%d", &value) != 1 || value < 1) {
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

	if(sscanf(str, "%lf", &value) != 1 || value < 0) {
		fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
		exit(1);
	}
	return value;
}


/*--------------------------------------------------------------------
| Function: main
---------------------------------------------------------------------*/

int main (int argc, char** argv) {

	if(argc != 7) {
		fprintf(stderr, "\nNumero invalido de argumentos.\n");
		fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf iteracoes\n\n");
		return 1;
	}

	/* argv[0] = program name */
	int N = parse_integer_or_exit(argv[1], "N");
	double tEsq = parse_double_or_exit(argv[2], "tEsq");
	double tSup = parse_double_or_exit(argv[3], "tSup");
	double tDir = parse_double_or_exit(argv[4], "tDir");
	double tInf = parse_double_or_exit(argv[5], "tInf");
	int iteracoes = parse_integer_or_exit(argv[6], "iteracoes");

	DoubleMatrix2D *matrix, *matrix_aux, *result;


	fprintf(stderr, "\nArgumentos:\n"
	" N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iteracoes=%d\n",
	N, tEsq, tSup, tDir, tInf, iteracoes);


	matrix = dm2dNew(N+2, N+2);
	matrix_aux = dm2dNew(N+2, N+2);


	/* FAZER ALTERACOES AQUI */
	//3
	if (N<1 || tEsq<0 || tSup<0 || tDir<0 || tInf<0 || iteracoes<1) {
		exit(1);
	};


	dm2dSetLineTo (matrix, 0, tSup);
	dm2dSetLineTo (matrix, N+1, tInf);
	dm2dSetColumnTo (matrix, 0, tEsq);
	dm2dSetColumnTo (matrix, N+1, tDir);
	/*int n;
	for (n = 0; n < N+2; n++) {
		dm2dSetLineTo (matrix, n, n);
	};*/


	dm2dCopy (matrix_aux, matrix);

	result = simul(matrix, matrix_aux, N+2, N+2, iteracoes);

	//dm2dPrint(result);

	dm2dFree(matrix);
	dm2dFree(matrix_aux);

	return 0;
}
