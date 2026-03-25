/*
 * Auteur(s): Samuel YAO
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_MAX    100000000

long double sum = 0.0;

void simple_sum() {
    unsigned long long n;
    long double sign = 1.0;

    for (n = 0; n < N_MAX; n++, sign = -sign)
        sum += sign / (2.0 * n + 1.0);
}

int main () {
    clock_t start_time = clock();
    simple_sum();

    printf("pi = %.20Lf\n", 4.0 * sum);

    clock_t end_time = clock();
    printf("Execution time: %f seconds\n", ((double)(end_time - start_time)) / CLOCKS_PER_SEC);

    exit(EXIT_SUCCESS);
}
