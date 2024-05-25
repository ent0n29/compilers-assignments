// Per produrre la IR che sarà l'input del nostro ottimizzatore:
//
// 	clang -O2 -S -emit-llvm -c FILENAME.c -o FILENAME.ll
//
// Per lanciare il nostro passo di analisi come unico passo dell'ottimizzatore:
//
//	opt -load-pass-plugin=lib/libTestPass.so -passes=test-pass -disable-output FILENAME.ll
//
// Il flag `-disable-output` evita la generazione di bytecode in output (non ci serve,
// il nostro passo non trasforma la IR e non genera output)
//

int g;
int loop(int **a, int **b, int **c, int N) {
    int i, j, ret=0;
    int d[N][N];

    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            a[i][j] = 1/b[i][j]*c[i][j];

    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            d[i][j] = a[i][j]+c[i][j];

    return ret + g;
}