#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

#define MFENCE() __asm__ volatile ("mfence" ::: "memory")

_Atomic int flag[2];
_Atomic int turn;
_Atomic int counter = 0;

static void lock_acquire(int self)
{
    int other = 1 - self;
    atomic_store_explicit(&flag[self], 1, memory_order_seq_cst);
    atomic_store_explicit(&turn, other, memory_order_seq_cst);
    while (atomic_load_explicit(&flag[other], memory_order_seq_cst) == 1 &&
           atomic_load_explicit(&turn, memory_order_seq_cst) == other)
        ;
    MFENCE();
}

static void lock_release(int self)
{
    atomic_store_explicit(&flag[self], 0, memory_order_seq_cst);
}

struct args { int id; int iterations; };

static void *worker(void *arg) {
    struct args *a = (struct args *)arg;
    for (int i = 0; i < a->iterations; i++) {
        lock_acquire(a->id);
        counter = counter + 1;
        lock_release(a->id);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uzycie: %s <cores>\n", argv[0]);
        return 1;
    }
    const char *cores = argv[1];

    const int ITERATIONS = 500000;
    pthread_t t0, t1;
    struct args a0 = {0, ITERATIONS};
    struct args a1 = {1, ITERATIONS};
    counter = 0;

    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    pthread_create(&t0, NULL, worker, &a0);
    pthread_create(&t1, NULL, worker, &a1);
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);

    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    double elapsed = (ts_end.tv_sec - ts_start.tv_sec)
                   + (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

    long long expected = 2LL * ITERATIONS;
    long long actual   = counter;
    long long lost     = expected - actual;

    printf("=== PETERSON POPRAWNA (_Atomic + mfence) ===\n");
    printf("Oczekiwano:  %lld\n", expected);
    printf("Otrzymano:   %lld\n", actual);
    printf("Utracone:    %lld\n", lost);
    printf("Czas:        %.6f s\n", elapsed);
    if (lost != 0)
        printf("WYNIK: RACE CONDITION\n");
    else
        printf("WYNIK: OK\n");

    FILE *f_actual = fopen("p_ok.csv", "a");
    if (f_actual == NULL) {
        fprintf(stderr, "Nie mozna otworzyc pliku: p_ok.csv\n");
        return 1;
    }
    fprintf(f_actual, "%s,%lld\n", cores, actual);
    fclose(f_actual);

    FILE *f_time = fopen("p_ok_time.csv", "a");
    if (f_time == NULL) {
        fprintf(stderr, "Nie mozna otworzyc pliku: p_ok_time.csv\n");
        return 1;
    }
    fprintf(f_time, "%s,%.6f\n", cores, elapsed);
    fclose(f_time);
    return 0;
}
