#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct rec {
    int key;
    int value[24]; // 96 bytes
} rec_t;

void merge (int low, int mid, int high)
{

}

void* mergesort (int low, int high)
{
    // calculating mid point of array
    int mid = low + (high - low) / 2;

    if (low < high) {
        // calling first half
        mergesort(low, mid);

        // calling second half
        mergesort(mid + 1, high);

        // merging the two halves
        merge(low, mid, high);
    }
}

void* mergesort_thread()
{

}

int main (int argc, char* argv[])
{
    // if (argc != 3)
    //     return 1;
    
    // Parsing through input file
    FILE *in;
    FILE *out;

    in = fopen(argv[1], "rb");
    if (in == NULL)
        return 1;

    // CHECKING FILE SIZE
    struct stat st;
    stat(argv[1], &st);
    int size = st.st_size;
    
    // char* record [size/100];

    rec_t recs[size/100];

    int record[size];
    fread(&record, sizeof(record), 1, in);
    for (int i = 0; i < 15; i++) {
        printf("bytes[%d]: %x\n", i, record[i] & 0xff);
    }


    /*
    // USING STRUCT AND FREAD
    struct rec *record;
    record = malloc(sizeof(struct rec) * (size/100));
    // int i = 0;

    fread(record, sizeof(struct rec), (size/100), in);
    printf("record size: %ld\n", sizeof(record));
    for (int i = 0; i < (size/100); i++)
        printf("Record key %i: %d\n", i, (record++)->key);
    // record++;
    // printf("Record key 2: %x\n", record->key); 
    // printf("Record key 2: %d\n", record->key);


    // while (1) {
    //     fread(&record, sizeof(struct rec), 1, in);
    //     if (fgetc(in) == EOF) {
    //         break;
    //     }
    //     printf("Record key %d: %d\n", i, record.key);
    //     recs[i] = record;
    //     i++;
    // }

    */

    

    // for (int i = 0; i < size/100; i++) {
    //     char k[5];    
    //     memcpy(k, &record[i], 4);
    //     k[4] = '\0';
    //     int key = *(int *) k;

    //     char value[24];
    //     memcpy(key, &record[i], 4);
    //     key[4] = '\0';


    // }
    
    // printf("%d\n", *(int *) &record[1]);


        // int value = *(int *)v;
        
        // rec_t r = { .key = key, .value = value};
        // recs[0] = r;
    // }

    // for (int i = 0; i < size; i++)
    //     printf("Key %d: %d\n", i+1, *(int *) record[i]);

    // Open threads
    // int numThreads = get_nprocs();
    // pthread_t tid[numThreads];
 
    // for (int i = 0; i < numThreads; i++)
    //     pthread_create(&tid[i], NULL, mergesort_thread, (void *)&tid);

    // for (int i = 0; i < numThreads; i++)
    //     pthread_join(tid[i], NULL);
 
    // pthread_exit(NULL);
    return 0;

}