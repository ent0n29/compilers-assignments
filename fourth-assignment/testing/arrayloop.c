void twoloop(int a[], int b[], int c[], int d[], int n) {
    int i;
    for (i=0; i<n; i++)
        a[i] = b[i] + c[i];

    for (i=0; i<n; i++)
        d[i] = a[i] * a[i];

    return ;
}