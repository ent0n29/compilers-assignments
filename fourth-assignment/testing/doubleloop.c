int loop(int a, int b) {
    int tot=0;

    for (int i=0; i<12; i++)
        tot += a;

    for (int i=0; i<12; i++)
        tot += b;

    return tot;
}