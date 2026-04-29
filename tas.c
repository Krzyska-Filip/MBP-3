/* ============================================================
   TAS LOCK (C11 + pthreads)
   Test-And-Set lock
   ============================================================
   Kompilacja: gcc -O2 -pthread tas.c -o tas.out
   Uzycie:     ./tas.out <cores>
   ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

_Atomic int flag = 0;
_Atomic long long counter = 0;

static void lock_acquire(void)
{
    /* while flag.TAS(∥) ; */
    while (atomic_exchange_explicit(&flag, 1, memory_order_relaxed))
        ;
    /* fence(R∥RW) */
    atomic_thread_fence(memory_order_seq_cst);
}

static void lock_release(void)
{
    /* flag.store(false, RW∥) */
    atomic_store_explicit(&flag, 0, memory_order_release);
}

struct args { int id; int iterations; };

static void *worker(void *arg)
{
    struct args *a = (struct args *)arg;
    for (int i = 0; i < a->iterations; i++) {
        lock_acquire();
	counter = counter + 1;
        lock_release();
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Uzycie: %s <threads>\n", argv[0]);
        return 1;
    }
    const char *cores = argv[1];
    int n = atoi(argv[1]);
    const int ITERATIONS = 1000000;

    pthread_t *threads = malloc(n * sizeof(pthread_t));
    struct args *args_arr = malloc(n * sizeof(struct args));
    counter = 0;

    int per_thread = ITERATIONS / n;
    int remainder  = ITERATIONS % n;

    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    for (int i = 0; i < n; i++) {
        args_arr[i].id = i;
        args_arr[i].iterations = per_thread + (i < remainder ? 1 : 0);
        pthread_create(&threads[i], NULL, worker, &args_arr[i]);
    }
    for (int i = 0; i < n; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    double elapsed = (ts_end.tv_sec  - ts_start.tv_sec)
                   + (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

    long long actual = counter;

    printf("=== TAS LOCK ===\n");
    printf("Threads:   %d\n", n);
    printf("Otrzymano: %lld / %d\n", actual, ITERATIONS);
    printf("Czas:      %.6f s\n", elapsed);

    FILE *f = fopen("tas_time.csv", "a");
    if (f == NULL) { fprintf(stderr, "Nie mozna otworzyc: tas_time.csv\n"); return 1; }
    fprintf(f, "%s,%.6f\n", cores, elapsed);
    fclose(f);

    free(threads);
    free(args_arr);

    printf("Zapisano do: tas_time.csv\n");
    return 0;
}
