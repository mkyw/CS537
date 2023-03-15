#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/sysinfo.h>

typedef struct rec
{
    int key;
    char pointer[96];
} rec_t;

typedef struct a1
{
    int l;
    int r;
    int p;
    struct rec_t* a;
} qs_param;

rec_t *c;
pthread_mutex_t lock;
int current = 0;
int num_threads;
int num_records = 0;

pthread_t threads[2];

void *thread_quicksort(void *arg)
{
    qs_param *qp;
    qp = (qs_param *) arg;
    int a = qp->a;
    int l = qp->l;
    int r = qp->r;
    int p = qp->p;
    
    int j;

    if( l < r ) 
    {
        j = partition(a, l, r);

        qs_param *qp_left;
        qp_left->a = a;
        qp_left->l = l;
        qp_left->r = j - 1;
        qp_left->p = p - 1;

        qs_param *qp_right;
        qp_right->a = a;
        qp_right->l = j + 1;
        qp_right->r = r;
        qp_right->p = p - 1;

        if(p > 0)
        {
            pthread_create(&thread[2*(p - 1)], NULL, thread_quicksort, (void*)%qp_left);
            pthread_create(&thread[2*(p - 1) + 1], NULL, thread_quicksort, (void*)%qp_left);
        }
        else
        {
            quicksort(a, l, j - 1, 0);
            quicksort(a, j + 1, r, 0);
        }
    }

}

int partition(struct rec* a, int l, int r)
{
    int pivot, i, j;
    struct rec t;
    pivot = a[l].key;
    i = l; j = r+1;

    while(1)
    {
        do ++i; while( a[i].key <= pivot && i <= r );
        do --j; while( a[j].key > pivot );
        if( i >= j ) break;
        t = a[i];
        a[i] = a[j];
        a[j] = t;
    }

    t = a[l];
    a[l] = a[j];
    a[j] = t;

    return j;
}

int main(int argc, char *argv[])
{

    FILE *in;
    in = fopen(argv[1], "rb");
    int fd = open(argv[1], O_RDONLY);
    if (in == NULL)
    {
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }
    int size = st.st_size;

    if (size == 0)
    {
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    c = (rec_t *)malloc(size * sizeof(rec_t));

    // char *map = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_NORESERVE, fd, 0);

    for (int i = 0; i < size / 100; i++)
    {
        fseek(in, i * 100, SEEK_SET);
        int r = fread(c, 100, 1, in);
        if (r < -1)
            exit(1);
        // fread(&(c->key),1,4,in);
        // fread(&(c->pointer),96,1,in);
        // memcpy(&(c->key), r, 400);
        // memcpy(&(c->pointer), (4+map+i*100), 96);
        // c->key = *(int16_t *) r;
        // printf("%d: %d\n", i, c->key);
        c++;
        num_records++;
    }
    c = c - size / 100;

    quicksort(c, 0, size/100);

    // num_threads = get_nprocs();
    // pthread_t threads[num_threads];
    // for (int i = 0; i < num_threads; i++)
    // {
    //     pthread_create(&threads[i], NULL, thread_mergesort, (void *)NULL);
    // }
    // for (int i = 0; i < num_threads; i++)
    // {
    //     pthread_join(threads[i], NULL);
    // }

    // mergesort(0, (size / 100) - 1);

    // int output = creat(argv[2], S_IWUSR | S_IRUSR);

    // for (int i = 0; i < size / 100; i++)
    // {
    //     if (write(output, (c + i), sizeof(struct rec)) < -1)
    //         exit(1);
    // }

    // fclose(in);
    // fsync(fd);
    // close(fd);

    // free(c);

    return 0;
}
