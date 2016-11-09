#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int
main(int argc, char **argv)
{
    int count = 1000000;
    int *ints = (int *) malloc(sizeof(int) * count);
    srand(13);
    for (int i = 0; i < count; i++) {
        ints[i] = rand();
    }
    /*  sprintf  */
    char buf[20];
    clock_t cs, ce;
    cs = clock();
    for (int i = 0; i < count; i++) {
        sprintf(buf, "%d", ints[i]);
    }
    ce = clock();
    printf("test #1 sprintf, clocks: %ld, sec: %f\n", (long) (ce - cs), (double) (ce - cs) / CLOCKS_PER_SEC);
    /*  own 1  */
    char num[20];
    cs = clock();
    for (int i = 0; i < count; i++) {
        char *k = num;
        char *p = buf;
        int n = ints[i];
        do {
            *k++ = n % 10 + '0';
        } while (n /= 10);
        while (k > num) {
            *p++ = *(--k);
        }
    }
    ce = clock();
    printf("test #2 own 1, clocks: %ld, sec: %f\n", (long) (ce - cs), (double) (ce - cs) / CLOCKS_PER_SEC);
    /*  own 2  */
    cs = clock();
    for (int i = 0; i < count; i++) {
        int count = 0;
        int n = ints[i];
        do {
            count++;
        } while (n /= 10);
        char *p = buf;
        n = ints[i];
        do {
            buf[count--] = n % 10 + '0';
        } while (n /= 10);
    }
    ce = clock();
    printf("test #3 own 2, clocks: %ld, sec: %f\n", (long) (ce - cs), (double) (ce - cs) / CLOCKS_PER_SEC);

}
