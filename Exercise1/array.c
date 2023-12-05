//AUTHOR: Artur Gil Torres, a.gtorres@udc.es

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <threads.h>
#include <unistd.h>
#include "options.h"

#define DELAY_SCALE 1000


struct array {
    int size;
    int *arr;
};

typedef struct{
    int id;
    int iterations;
    int delay;
    struct array arr;
    mtx_t* mutex;
}thr_args;


void apply_delay(int delay) {
    for(int i = 0; i < delay * DELAY_SCALE; i++); // waste time
}


int increment(void* p)
{
    thr_args *args = (thr_args*) p;    

    int pos, val;

    for(int i = 0; i < args->iterations; i++) {

        mtx_lock(args->mutex); // <----- protect array locking mutex

        pos = rand() % args->arr.size;

        printf("Thread %d increasing position %d\n", args->id, pos);

        val = args->arr.arr[pos];   //read
        apply_delay(args->delay);

        val ++;                     //increment
        apply_delay(args->delay);

        args->arr.arr[pos] = val;   //save back
        apply_delay(args->delay);

        mtx_unlock(args->mutex); // <----- unlock mutex
    }
    free(args);

    return 0;
}


void print_array(struct array arr) {
    int total = 0;

    for(int i = 0; i < arr.size; i++) {
        total += arr.arr[i];
        printf("%d ", arr.arr[i]);
    }

    printf("\nTotal: %d\n", total);
}


int main (int argc, char **argv)
{
    struct options       opt;
    struct array         arr;

    srand(time(NULL));

    // Default values for the options. Use command options instead of changing this values.

    opt.num_threads  = 2;
    opt.size         = 10;
    opt.iterations   = 100;
    opt.delay        = 1000;

    read_options(argc, argv, &opt);

    arr.size = opt.size;
    arr.arr  = malloc(arr.size * sizeof(int));

    memset(arr.arr, 0, arr.size * sizeof(int));

    thrd_t *th;
    thr_args *argum;
    mtx_t mutex;
    
    th = malloc(opt.num_threads * sizeof(thrd_t));
    mtx_init(&mutex, mtx_plain);

    for(int i = 0; i < opt.num_threads; i++){
        argum = malloc(sizeof(thr_args));
        argum->id = i;
        argum->iterations = opt.iterations;
        argum->delay = opt.delay;
        argum->arr = arr; 
        argum->mutex = &mutex;
        if (thrd_create(&th[i], increment, argum) != 0) perror("Error creating thread");
    }

    for(int i = 0; i < opt.num_threads; i++){
        if (thrd_join(th[i], NULL) !=0 ) perror("Error joining thread");
    }

    print_array(arr);

    mtx_destroy(&mutex);
    free(th);
    free(arr.arr);

    return 0;
}