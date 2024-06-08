int loop(int a, int b, int N) {
    int tot=0, tot1=0;

    for (int i=1; i<N; i++)
        tot += a;

    for (int i=1; i<N; i++)
        tot1 += b;

    return tot;
}