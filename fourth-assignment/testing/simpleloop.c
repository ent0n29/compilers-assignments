int loop(int a, int b) {
    int i=0;
    int tot= 100;
    int tot1= 3;
    int n = 10;

    for (i=0; i<n; i++)
        tot += a;

    tot=0;
    for (i=0; i<n; i++)
        tot1 += b;
    
    for (i=0; i<n; i++)
        tot+= a;

    return tot+tot1;
}