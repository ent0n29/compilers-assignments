/**
 * This piece of code is supposed
 * to be translated in LLVM IR
 * as a pair of guarded loops,
 * since the UB is unknown and the
 * start value is not guaranteed
 * to satisfy the condition at
 * the first iteration.
*/

int loop(int a, int b, int N) {
    int tot=0, tot1=0;

    for (int i=1; i<N; i++)
        tot += a*i;

    for (int i=1; i<N; i++)
        tot1 += b*i;

    return tot + tot1;
}