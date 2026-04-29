#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

volatile int flag[2] = {0, 0};
volatile int turn    = 0;
long long counter = 0;

static void peterson_enter(int me) {
    int other = 1 - me;
    flag[me] = 1;
    turn     = other;
    while (flag[other] && turn == other)
        ;
}

static void peterson_exit(int me) {
    flag[me] = 0;
}

struct args { int id; int iterations; };

static void *worker(void *arg) {
    struct args *a = (struct args *)arg;
    for (int i = 0; i < a->iterations; i++) {
        peterson_enter(a->id);
        counter = counter + 1;
        peterson_exit(a->id);
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

    printf("=== PETERSON ZEPSUTA (volatile bez barier) ===\n");
    printf("Oczekiwano:  %lld\n", expected);
    printf("Otrzymano:   %lld\n", actual);
    printf("Utracone:    %lld\n", lost);
    printf("Czas:        %.6f s\n", elapsed);
    if (lost != 0)
        printf("WYNIK: RACE CONDITION\n");
    else
        printf("WYNIK: OK\n");

    //
    // Zapisywanie wyników do pliku
    //
    FILE *f_actual = fopen("p_br.csv", "a");
    if (f_actual == NULL) {
        fprintf(stderr, "Nie mozna otworzyc pliku: p_br.csv\n");
        return 1;
    }
    fprintf(f_actual, "%lld\n", actual);
    fclose(f_actual);

    FILE *f_time = fopen("p_br_time.csv", "a");
    if (f_time == NULL) {
        fprintf(stderr, "Nie mozna otworzyc pliku: p_br_time.csv\n");
        return 1;
    }
    fprintf(f_time, "%s,%.6f\n", argv[1], elapsed);
    fclose(f_time);
    return 0;
}
