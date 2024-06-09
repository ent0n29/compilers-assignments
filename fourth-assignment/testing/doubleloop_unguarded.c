/**
 * This piece of code is supposed
 * to be translated in LLVM IR
 * as a pair of unguarded loops,
 * since the UB is known and the
 * start value is guaranteed by
 * i = 1 initialization.
*/

int loop(int a, int b) {
    int tot=0, tot1=0;

    for (int i=1; i<12; i++)
        tot += a*i;

    for (int i=1; i<12; i++)
        tot1 += b*i;

    return tot + tot1;
}