void twoloop(int a[], int b[], int n) {
  int  t = 0;

    for (int i=0; i<n; ++i) {
        a[i+1] = t;
    }

    for (int i=0; i<n; ++i) {
        t = a[i];
    }


    return;
}