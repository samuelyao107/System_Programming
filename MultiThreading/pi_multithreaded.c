/*
 * Auteur(s): Samuel YAO
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bits/pthreadtypes.h>

#define N_MAX    100000000

long double sum = 0.0;

struct thread_data {
    unsigned long long start;
    unsigned long long end;
};

void * simple_sum(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    unsigned long long n;
    long double sign = 1.0;
    long double *thread_sum = malloc(sizeof(long double));
    if (data->start % 2 == 1) {
        sign = -1.0; 
    }
    for (n = data->start; n <= data->end; n++, sign = -sign)
        *thread_sum += sign / (2.0 * n + 1.0);
    return thread_sum;
}

int main () {

    /*On utilise quatre threads pour calculer la somme car mon processeur a quatre cœurs*/
    pthread_t thread1, thread2, thread3, thread4;
    struct thread_data thread1_data = {0, 24999999};
    struct thread_data thread2_data = {25000000, 49999999};
    struct thread_data thread3_data = {50000000, 74999999};
    struct thread_data thread4_data = {75000000, 99999999};
    long double *result1, *result2, *result3, *result4;
  
    clock_t start_time = clock();
    if( pthread_create(&thread1, NULL, simple_sum, &thread1_data) != 0 ) {
        perror("Failed to create thread1");
        exit(EXIT_FAILURE);
    }
    if( pthread_create(&thread2, NULL, simple_sum, &thread2_data) != 0 ) {
        perror("Failed to create thread2");
        exit(EXIT_FAILURE);
    }
    if( pthread_create(&thread3, NULL, simple_sum, &thread3_data) != 0 ) {
        perror("Failed to create thread3");
        exit(EXIT_FAILURE);
    }
    if( pthread_create(&thread4, NULL, simple_sum, &thread4_data) != 0 ) {
        perror("Failed to create thread4");
        exit(EXIT_FAILURE);
    }
    
    if( pthread_join(thread1, &result1) == 0 ) {
        sum += *result1;
        free(result1);
    }
    if( pthread_join(thread2, &result2) == 0 ) {
        sum += *result2;
        free(result2);
    }
    if( pthread_join(thread3, &result3) == 0 ) {
        sum += *result3;
        free(result3);
    }
    if( pthread_join(thread4, &result4) == 0 ) {
        sum += *result4;
        free(result4);
    }   
   


    printf("pi = %.20Lf\n", 4.0 * sum);

    clock_t end_time = clock();
    printf("Execution time: %f seconds\n", ((double)(end_time - start_time)) / CLOCKS_PER_SEC);

    exit(EXIT_SUCCESS);
}
