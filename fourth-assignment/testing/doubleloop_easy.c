int loop(int a, int b) {
    int tot=0, tot1=0;

    for (int i=1; i<12; i++)
        tot += a;

    for (int i=1; i<12; i++)
        tot1 += b;

    return tot;
}