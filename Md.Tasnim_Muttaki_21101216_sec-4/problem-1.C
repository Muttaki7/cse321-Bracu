#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

typedef struct
{
    int n;
    int64_t *fib_seq;
} FibArgs;

typedef struct
{
    int *search_indices;
    int s;
    int64_t *fib_seq;
    int n;
    int64_t *results;
} SearchArgs;

void *generate_fibonacci(void *arg)
{
    FibArgs *args = (FibArgs *)arg;
    int n = args->n;
    int64_t *fib = args->fib_seq;

    if (n >= 0)
        fib[0] = 0;
    if (n >= 1)
        fib[1] = 1;
    for (int i = 2; i <= n; i++)
    {
        fib[i] = fib[i - 1] + fib[i - 2];
    }
    return NULL;
}

void *search_fibonacci(void *arg)
{
    SearchArgs *args = (SearchArgs *)arg;
    int *indices = args->search_indices;
    int s = args->s;
    int64_t *fib = args->fib_seq;
    int n = args->n;
    int64_t *results = args->results;

    for (int i = 0; i < s; i++)
    {
        if (indices[i] >= 0 && indices[i] <= n)
        {
            results[i] = fib[indices[i]];
        }
        else
        {
            results[i] = -1;
        }
    }
    return NULL;
}

int main()
{
    int n, s;
    printf("Enter the term of fibonacci sequence: ");
    scanf("%d", &n);

    if (n < 0 || n > 40)
    {
        printf("n must be between 0 and 40\n");
        return 1;
    }

    printf("How many numbers you are willing to search?: ");
    scanf("%d", &s);

    if (s <= 0)
    {
        printf("Number of searches must be greater than 0\n");
        return 1;
    }

    int64_t *fib_seq = (int64_t *)malloc((n + 1) * sizeof(int64_t));
    if (!fib_seq)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    int *search_indices = (int *)malloc(s * sizeof(int));
    int64_t *search_results = (int64_t *)malloc(s * sizeof(int64_t));
    if (!search_indices || !search_results)
    {
        printf("Memory allocation failed\n");
        free(fib_seq);
        free(search_indices);
        free(search_results);
        return 1;
    }

    for (int i = 0; i < s; i++)
    {
        printf("Enter search %d: ", i + 1);
        scanf("%d", &search_indices[i]);
    }

    FibArgs fib_args = {n, fib_seq};
    SearchArgs search_args = {search_indices, s, fib_seq, n, search_results};

    pthread_t fib_thread, search_thread;

    if (pthread_create(&fib_thread, NULL, generate_fibonacci, &fib_args))
    {
        printf("Failed to create Fibonacci thread\n");
        free(fib_seq);
        free(search_indices);
        free(search_results);
        return 1;
    }

    pthread_join(fib_thread, NULL);

    for (int i = 0; i <= n; i++)
    {
        printf("a[%d]=%lld\n", i, fib_seq[i]);
    }

    if (pthread_create(&search_thread, NULL, search_fibonacci, &search_args))
    {
        printf("Failed to create search thread\n");
        free(fib_seq);
        free(search_indices);
        free(search_results);
        return 1;
    }

    pthread_join(search_thread, NULL);

    for (int i = 0; i < s; i++)
    {
        printf("result of search #%d = %lld\n", i + 1, search_results[i]);
    }

    free(fib_seq);
    free(search_indices);
    free(search_results);

    return 0;
}