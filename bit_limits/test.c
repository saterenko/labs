#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Judy.h>

int
main(int argc, char **argv)
{
    int campaigns = 1000000;
    int nels = 10;
    int *values = (int *) malloc(sizeof(int) * campaigns * nels);
    if (!values) {
        printf("error in %d\n", __LINE__);
        exit(1);
    }
    srand(13);
    for (int i = 0; i < campaigns; i++) {
        for (int j = 0; j < nels; j++) {
            values[i * nels + j] = rand() % 256;
        }
    }

    /*  test 1, bruteforce  */
    clock_t cs, ce;
    int found_campaigns = 0;
    int reference = 13;
    cs = clock();
    for (int i = 0; i < campaigns; i++) {
        for (int j = 0; j < nels; j++) {
            if (values[i * nels + j] == reference) {
                found_campaigns++;
                break;
            }
        }
    }
    ce = clock();
    printf("test #1 bruteforce, found: %d, clocks: %ld, sec: %f\n", found_campaigns, (long) (ce - cs), (double) (ce - cs) / CLOCKS_PER_SEC);

    /*  test 2, set  */
    Pvoid_t *sets = (Pvoid_t) malloc(sizeof(Pvoid_t) * campaigns);
    if (!sets) {
        printf("error in %d\n", __LINE__);
        exit(1);
    }
    memset(sets, 0, sizeof(Pvoid_t) * campaigns);
    for (int i = 0; i < campaigns; i++) {
        for (int j = 0; j < nels; j++) {
            PWord_t pw;
            JLI(pw, sets[i], values[i * nels + j]);
            if (pw == PJERR) {
                printf("error in %d\n", __LINE__);
                exit(1);
            }
        }
    }
    found_campaigns = 0;
    cs = clock();
    for (int i = 0; i < campaigns; i++) {
        PWord_t pw;
        JLG(pw, sets[i], reference);
        if (pw) {
            found_campaigns++;
        }
    }
    ce = clock();
    printf("test #2 set, found: %d, clocks: %ld, sec: %f\n", found_campaigns, (long) (ce - cs), (double) (ce - cs) / CLOCKS_PER_SEC);
    for (int i = 0; i < campaigns; i++) {
        int rc;
        JLFA(rc, sets[i]);
    }

    /*  test 3, bitmaps  */
    #define SET_BIT(_n, _i) _n[_i / 64] |= 1ULL << (_i % 64)
    #define CHECK_BIT(_n, _i) _n[_i / 64] & (1ULL << (_i % 64))

    PWord_t pw;
    Pvoid_t pv = (Pvoid_t) 0;
    int mask_size = sizeof(uint64_t) * (campaigns / 64 + 1);
    for (int i = 0; i < campaigns; i++) {
        for (int j = 0; j < nels; j++) {
            int id = values[i * nels + j];
            JLI(pw, pv, id);
            if (pw == PJERR) {
                printf("error in %d\n", __LINE__);
                exit(1);
            } else if (*pw) {
                uint64_t *mask = (uint64_t *) *pw;
                SET_BIT(mask, i);
            } else {
                uint64_t *mask = (uint64_t *) malloc(mask_size);
                if (!mask) {
                    printf("error in %d\n", __LINE__);
                    exit(1);
                }
                memset(mask, 0, mask_size);
                SET_BIT(mask, i);
                *pw = (Word_t) mask;
            }
        }
    }
    found_campaigns = 0;
    cs = clock();
    JLG(pw, pv, reference);
    if (pw) {
        uint64_t *mask = (uint64_t *) *pw;
        for (int i = 0; i < campaigns; i++) {
            if (CHECK_BIT(mask, i)) {
                found_campaigns++;
            }
        }
    }
    ce = clock();
    printf("test #3 bitmaps, found: %d, clocks: %ld, sec: %f\n", found_campaigns, (long) (ce - cs), (double) (ce - cs) / CLOCKS_PER_SEC);

    /*  test 4, bitmaps 2 */
    found_campaigns = 0;
    cs = clock();
    JLG(pw, pv, reference);
    if (pw) {
        uint64_t *mask = (uint64_t *) *pw;
        for (int i = 0; i < (campaigns / 64 + 1); i++) {
            uint64_t m = mask[i];
            while (m) {
                int n = __builtin_ffsll(m);
                m &= ~(1ULL << (n - 1));
                found_campaigns++;
            }
        }
    }
    ce = clock();
    printf("test #4 bitmaps 2, found: %d, clocks: %ld, sec: %f\n", found_campaigns, (long) (ce - cs), (double) (ce - cs) / CLOCKS_PER_SEC);

    Word_t index = 0;
    JLF(pw, pv, index);
    while (pw) {
        if (*pw) {
            free((void *) *pw);
        }
        JLN(pw, pv, index);
    }
    int rc;
    JLFA(rc, pv);
    free(values);

    return 0;
}
