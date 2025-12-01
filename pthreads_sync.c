#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

// Κοινόχρηστες μεταβλητές
long long shared_counter = 0;
int iterations_per_thread = 0;

// Μηχανισμοί Συγχρονισμού
pthread_mutex_t mutex_lock;
pthread_rwlock_t rwlock;


// 1.Προσέγγιση με Mutex
void* worker_mutex(void* arg) {
    for (int i = 0; i < iterations_per_thread; i++) {
        pthread_mutex_lock(&mutex_lock);
        shared_counter++;
        pthread_mutex_unlock(&mutex_lock);
    }
    return NULL;
}

// 2.Προσέγγιση με Read-Write Lock
// Απαιτείται αποκλειστικό κλείδωμα (Write Lock) λόγω ενημέρωσης της μεταβλητής
void* worker_rwlock(void* arg) {
    for (int i = 0; i < iterations_per_thread; i++) {
        pthread_rwlock_wrlock(&rwlock);
        shared_counter++;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

// 3.Προσέγγιση με Atomic Built-ins
void* worker_atomic(void* arg) {
    for (int i = 0; i < iterations_per_thread; i++) {
        // __atomic_fetch_add επιστρέφει την παλιά τιμή, αλλά αυξάνει ατομικά τη μεταβλητή
        // __ATOMIC_SEQ_CST για αυστηρή σειριακή συνέπεια μνήμης
        __atomic_fetch_add(&shared_counter, 1, __ATOMIC_SEQ_CST);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <num_threads> <iterations> <mode: mutex|rwlock|atomic>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    iterations_per_thread = atoi(argv[2]);
    char* mode = argv[3];

    pthread_t threads[num_threads];
    void* (*thread_func)(void*) = NULL;

    // Αρχικοποίηση και επιλογή mode
    if (strcmp(mode, "mutex") == 0) {
        pthread_mutex_init(&mutex_lock, NULL);
        thread_func = worker_mutex;
    } else if (strcmp(mode, "rwlock") == 0) {
        pthread_rwlock_init(&rwlock, NULL);
        thread_func = worker_rwlock;
    } else if (strcmp(mode, "atomic") == 0) {
        thread_func = worker_atomic;
    } else {
        fprintf(stderr, "Invalid mode. Choose: mutex, rwlock, atomic\n");
        return 1;
    }

    // Μέτρηση Χρόνου
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Δημιουργία Νημάτων
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            perror("pthread_create failed");
            return 1;
        }
    }

    // Αναμονή Νημάτων
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Υπολογισμός χρόνου
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (strcmp(mode, "mutex") == 0) pthread_mutex_destroy(&mutex_lock);
    if (strcmp(mode, "rwlock") == 0) pthread_rwlock_destroy(&rwlock);

    printf("%s,%d,%d,%lld,%.6f\n", mode, num_threads, iterations_per_thread, shared_counter, elapsed);

    return 0;
}