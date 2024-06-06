void twoloop(int a[], int b[], int n) {
    for (int i=0; i<n+2; ++i) {
        a[i] = 5;
        int t = a[i+2];
    }

    for (int i=0; i<n+3; ++i) {
        a[i] = 5;
        int t = a[i];
    }

    for (int i=0; i<n+3; ++i) {
        int t = a[i];
        a[i+3] = 5;
    }

    return;
}