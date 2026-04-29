#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define TOTAL_ITERATIONS 1000000

#define MFENCE() __asm__ volatile ("mfence" ::: "memory")

_Atomic int flag[2];
_Atomic int turn;
_Atomic long long remaining;

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

struct args { int id; long long acquisitions; };

static void *worker(void *arg)
{
    struct args *a = (struct args *)arg;
    a->acquisitions = 0;
    while (1) {
        lock_acquire(a->id);
        if (remaining <= 0) {
            lock_release(a->id);
            break;
        }
        remaining--;
        a->acquisitions++;
        lock_release(a->id);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Uzycie: %s <cores>\n", argv[0]);
        return 1;
    }
    const char *cores = argv[1];

    remaining = TOTAL_ITERATIONS;

    pthread_t t0, t1;
    struct args a0 = {0, 0};
    struct args a1 = {1, 0};

    pthread_create(&t0, NULL, worker, &a0);
    pthread_create(&t1, NULL, worker, &a1);
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);

    double fairness = (a0.acquisitions == 0 || a1.acquisitions == 0)
                    ? 0.0
                    : (double)a0.acquisitions / (double)a1.acquisitions;

    printf("=== PETERSON FAIRNESS ===\n");
    printf("Cores:       %s\n", cores);
    printf("Wejscia T0:  %lld\n", a0.acquisitions);
    printf("Wejscia T1:  %lld\n", a1.acquisitions);
    printf("Fairness:    %.6f (idealnie 1.0)\n", fairness);

    FILE *f = fopen("p_ok_fairness.csv", "a");
    if (f == NULL) { 
        fprintf(stderr, "Nie mozna otworzyc: p_ok_fairness.csv\n"); 
        return 1; 
    }
    fprintf(f, "%s,%lld,%lld,%.6f\n", cores, a0.acquisitions, a1.acquisitions, fairness);
    fclose(f);
    return 0;
}
