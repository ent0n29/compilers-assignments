int loop(int a, int b) {
    int i=0;
    int tot= 100;
    int tot1= 3;

    for (i=0; i<12; i++)
        tot += a;

    tot1+= 4;
    for (i=0; i<12; i++)
        tot1 += b;
    
    for (i=0; i<12; i++)
        tot+= a;

    return tot+tot1;
}