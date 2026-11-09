#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_SIZE 100
#define NUM_THREADS 10

int matA[MAX_SIZE][MAX_SIZE];
int matB[MAX_SIZE][MAX_SIZE];

int matSumResult[MAX_SIZE][MAX_SIZE];
int matDiffResult[MAX_SIZE][MAX_SIZE];
int matProductResult[MAX_SIZE][MAX_SIZE];

int MAX = 20;

void fillMatrix(int matrix[MAX_SIZE][MAX_SIZE]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

void printMatrix(int matrix[MAX_SIZE][MAX_SIZE]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

static void sliceBounds(int threadNum, int* start, int* end) {
    *start = threadNum * MAX / NUM_THREADS;
    *end = (threadNum + 1) * MAX / NUM_THREADS;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the sum of the
// values at the coordinates of matA and matB.
void* computeSum(void* args) { // pass in the number of the ith thread
    int start, end;
    sliceBounds(*(int*)args, &start, &end);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the difference of the
// values at the coordinates of matA and matB.
void* computeDiff(void* args) { // pass in the number of the ith thread
    int start, end;
    sliceBounds(*(int*)args, &start, &end);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the inner product
// of matA and matB.
void* computeProduct(void* args) { // pass in the number of the ith thread
    int start, end;
    sliceBounds(*(int*)args, &start, &end);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            int sum = 0;
            for (int k = 0; k < MAX; k++) {
                sum += matA[i][k] * matB[k][j];
            }
            matProductResult[i][j] = sum;
        }
    }
    return NULL;
}

void* computeAll(void* args) {
    computeSum(args);
    computeDiff(args);
    computeProduct(args);
    free(args);
    return NULL;
}

// Spawn a thread to fill each cell in each result matrix.
// How many threads will you spawn?
int main(int argc, char* argv[]) {
    srand(time(0));  // Do Not Remove. Just ignore and continue below.

    // 0. Get the matrix size from the command line and assign it to MAX
    if (argc > 1) {
        MAX = atoi(argv[1]);
        if (MAX <= 0 || MAX > MAX_SIZE) {
            fprintf(stderr, "Matrix size must be between 1 and %d\n", MAX_SIZE);
            return 1;
        }
    }

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Create pthread_t objects for our threads.
    pthread_t threads[NUM_THREADS];

    // 4. Create 10 threads, each computing a slice of rows for every matrix operation.
    int created = 0;
    int failed = 0;
    for (int t = 0; t < NUM_THREADS; t++) {
        int* threadNum = malloc(sizeof(int));
        if (threadNum == NULL) {
            fprintf(stderr, "Failed to allocate memory for thread %d\n", t);
            failed = 1;
            break;
        }
        *threadNum = t;
        int rc = pthread_create(&threads[t], NULL, computeAll, threadNum);
        if (rc != 0) {
            fprintf(stderr, "Failed to create thread %d (error code %d)\n", t, rc);
            free(threadNum);
            failed = 1;
            break;
        }
        created++;
    }

    // 5. Wait for all threads to finish.
    for (int t = 0; t < created; t++) {
        int rc = pthread_join(threads[t], NULL);
        if (rc != 0) {
            fprintf(stderr, "Failed to join thread %d (error code %d)\n", t, rc);
            failed = 1;
        }
    }

    if (failed) {
        return 1;
    }

    // 6. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);
    return 0;
}