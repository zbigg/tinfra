#include <pthread.h>
#include <stdio.h>

int var = 0;

void* child_fn ( void* arg ) {
   __sync_add_and_fetch(&var, 1);
   return NULL;
}

int main ( void ) {
   pthread_t child;
   pthread_create(&child, NULL, child_fn, NULL);
   __sync_add_and_fetch(&var, 1);
   pthread_join(child, NULL);
   printf("result %i, expected 2\n", var);
   return 0;
}
