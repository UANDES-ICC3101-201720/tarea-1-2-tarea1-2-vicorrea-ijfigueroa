#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "types.h"
#include "const.h"
#include "util.h"



typedef struct {
	UINT *arreglo;
	int lo;
	int hi;
	UINT pivot;
} arguments;

void swap(UINT *a, UINT *b){
	UINT temp = *a;
	*a=*b;
	*b=temp;
}

int partition(UINT  *A, int lo, int hi){
	int pivot = A[hi];
	int i = (lo-1);

	for(int j = lo; j<=hi-1; j++){
		if(A[j]<= pivot){
			i++;
			swap(&A[i], &A[j]);
		}
	}
	swap(&A[i+1], &A[hi]);
	return (i+1);
}

// TODO: implement
void quicksort(UINT* A, int lo, int hi) {
	if(lo<hi){
		int pi = partition(A, lo, hi);
		quicksort(A, lo, pi-1);
		quicksort(A, pi+1, hi);
	}

}

// TODO: implement

void * parallel_partition(void *args){
	arguments *argumentos = args;
	UINT *A = argumentos->arreglo;
	for (UINT *z = A; z<A+64; z++){
		printf("%d,",*z);
	}
	int lo = argumentos->lo;
	int hi = argumentos->hi;
	printf("Lohi: %d, %d\n",lo, hi);
	UINT pivot = argumentos -> pivot;
	int i = (lo-1);
	for(int j = lo; j<=hi; j++){
		if(A[j] <= pivot){
			i++;
			swap(&A[i], &A[j]);
		}
	}
	
	printf("i=%d\n",i);
    size_t v = (size_t)i;

    return (void*) v;
	
}

void parallel_quicksort(UINT* A, int lo, int hi) {
    int n = hi - lo;
    printf("Arreglo de largo %d\n", n);
    //int cantidad_threads = 2*sysconf(_SC_NPROCESSORS_ONLN);
    int cantidad_threads = 16;
    int sub_block_size = n/cantidad_threads;
    int cant_bloque_sobrante = n - sub_block_size*cantidad_threads;
    int extra = 0;
    int inicios[cantidad_threads];
    int finales[cantidad_threads];
    for (int i = 0; i < cantidad_threads; i++){       
        printf("i = %d, extra = %d\n", i, extra);
        if (i < cant_bloque_sobrante){            
            //printf("Creando el thread numero %d que ve desde el %d hasta el %d\n", i, (sub_block_size*i)+extra, (sub_block_size*(i+1))+extra);
            inicios[i] = (sub_block_size*i)+extra;
            finales[i] = (sub_block_size*(i+1))+extra;
            extra ++;
            }
        else{
            //printf(">> Creando el thread numero %d que ve desde el %d hasta el %d\n", i, (sub_block_size*i)+extra, (sub_block_size*(i+1))+extra-1);
            inicios[i] = (sub_block_size*i)+extra;
            finales[i] = (sub_block_size*(i+1))+extra-1;
        }
        printf("%d\n", inicios[i]);
        printf("%d\n", finales[i]);
    }

    UINT pivot = (rand() % (hi-lo)) + lo;
    //UINT pivot = 0;
    printf("pivot: %u\n", A[pivot]);
    pthread_t threads[cantidad_threads]; 

    int Si[cantidad_threads];
    int Li[cantidad_threads];

    for (int i = 0; i<cantidad_threads; i++){
    	arguments *argumentos = malloc(sizeof(argumentos));
    	argumentos->arreglo = A;
    	argumentos->lo = inicios[i];
    	argumentos->hi = finales[i];
    	argumentos->pivot = A[pivot];

        printf("inicios[%d] = %d, finales[%d] = %d\n", i, inicios[i], i, finales[i]);
    	
        int * respuesta;
        
    	int ver = pthread_create(&threads[i], NULL, parallel_partition, argumentos);
    	pthread_join(threads[i], (void **)&respuesta);

        printf("respuesta = %d\n", (int)(intptr_t)respuesta);

        Si[i] = (int)(intptr_t)respuesta - inicios[i]+1;
        Li[i] = finales[i] - (int)(intptr_t)respuesta;

    	if(ver){
    		free(argumentos);
    	}
    }

    for (int u = 0; u < cantidad_threads; u++){
            printf("Si[%d] = %d\n", u, Si[u]);
            printf("Li[%d] = %d\n", u, Li[u]);
        }
    printf("n=%d\n",n );
    UINT index_partitions[cantidad_threads];
    index_partitions[0]=lo;
    for (UINT i = 1; i<cantidad_threads; i++){
    	index_partitions[i]=Si[i]+Li[i]+index_partitions[i-1];
    	printf("Index[%u]\n",index_partitions[i]);
    }
    UINT Qi = 0;
    UINT Ri = 0;
    //UINT A_prima[n];
    for (int *ind=Si; ind<Si+cantidad_threads;ind++){
    	Qi=Qi+*ind;

    }
    for (int *ind=Li; ind<Li+cantidad_threads; ind++){
    	Ri=Ri+*ind;
    }
    
    
    UINT *A_prima = malloc(n*sizeof(UINT));
    UINT *ind = A_prima;
    for(int i = 0; i<cantidad_threads; i++){
    	for(int j = index_partitions[i]; j<index_partitions[i]+Si[i];j++){
    		*ind = A[j];
    		ind++;
    	}
    }
    for(int i = 0; i<cantidad_threads; i++){
    	for(int j = index_partitions[i]+Si[i]; j<index_partitions[i]+Si[i]+Li[i];j++){
    		*ind = A[j];
    		ind++;
    	}

    }

    int cont = 0;
    for (UINT *ind = A_prima; ind<A_prima +n; ind++){
    	printf("A[%d] = %u\n", cont, *ind);
    	cont++;
    }







}

int main(int argc, char** argv) {
    srand (getpid());
    printf("[quicksort] Starting up...\n");

    /* Get the number of CPU cores available */
    printf("[quicksort] Number of cores available: '%ld'\n",
           sysconf(_SC_NPROCESSORS_ONLN));

    /* TODO: parse arguments with getopt */
    int experiments = 0;
    int t = 0;
    int c;
    char charT[2]= "";

    while((c = getopt (argc, argv, "T:E:")) !=-1){
        switch(c){
            case 'T':
            strcpy(charT, optarg);
            t = atoi(optarg);
            if(t < 1 || t > 9){
                fprintf(stderr, "%s\n", "T must be betweeen 3 and 9");
                return 0;
            }
            
            case 'E':
            experiments = atoi(optarg);
            if(experiments < 1){
                fprintf(stderr, "%s\n","Number of experiments must be greater or equal than 1" );
                return 0;
            }
        }
    }
    printf("T: %d, E: %d\n", t, experiments);



    /* TODO: start datagen here as a child process. */
    int dtgnid = fork();

    if(dtgnid == 0){
        printf("%s%d\n","Datagen PID: ", getpid());
        char * myargs[3];
        myargs[0] = "./datagen";
        myargs[1] = "inicio";
        myargs[2] = NULL;
        execvp(myargs[0], myargs);
    }
    else if(dtgnid>0){
        printf("%s%d\n", "Quicksort PID: ", getpid());
    }

    else if (dtgnid<0){
        fprintf(stderr, "%s\n", "Can't create Datagen as child process");
    }

    /* Create the domain socket to talk to datagen. */
    struct sockaddr_un addr;
    int fd;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("[quicksort] Socket error.\n");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, DSOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("[quicksort] connect error.\n");
        close(fd);
        exit(-1);
    }

    /* DEMO: request two sets of unsorted random numbers to datagen */
    for (int i = 0; i < experiments; i++) {
        /* T value 3 hardcoded just for testing. */
        char begin[] = "BEGIN U ";
        strcat(begin,charT);
        int rc = strlen(begin);

        /* Request the random number stream to datagen */
        if (write(fd, begin, strlen(begin)) != rc) {
            if (rc > 0) fprintf(stderr, "[quicksort] partial write.\n");
            else {
                perror("[quicksort] write error.\n");
                exit(-1);
            }
        }

        /* validate the response */
        char respbuf[10];
        read(fd, respbuf, strlen(DATAGEN_OK_RESPONSE));
        respbuf[strlen(DATAGEN_OK_RESPONSE)] = '\0';

        if (strcmp(respbuf, DATAGEN_OK_RESPONSE)) {
            perror("[quicksort] Response from datagen failed.\n");
            close(fd);
            exit(-1);
        }

        UINT readvalues = 0;
        size_t numvalues = pow(10, t);
        size_t readbytes = 0;

        UINT *readbuf = malloc(sizeof(UINT) * numvalues);

        while (readvalues < numvalues) {
            /* read the bytestream */
            readbytes = read(fd, readbuf + readvalues, sizeof(UINT) * 1000);
            readvalues += readbytes / 4;
        }
        printf("E%d:\n", i);
        /*for (UINT *pv = readbuf; pv < readbuf + numvalues; pv++) {
            printf("%u\n", *pv);
        }*/
        // Quicksorting
		struct timespec start, finish;
		double elapsed = 0;

		/* Get the wall clock time at start */
		clock_gettime(CLOCK_MONOTONIC, &start);

		//quicksort(readbuf, 0, numvalues);
		UINT arr[64] = {7, 13, 18, 2, 17,22, 76, 87, 98, 76, 54, 65, 45,83, 65, 43,15, 39, 1, 14, 20, 6, 10, 15, 9, 43, 34,23,11,22,33, 43, 47, 48, 55, 77, 3, 16, 19, 4, 11, 12, 5, 8, 12, 34, 52, 64, 34, 45, 23, 89, 99, 45, 24, 23, 54, 76, 59, 76, 53, 65, 45, 87};
        parallel_quicksort(arr, 0, 64);
		//parallel_quicksort(readbuf, 0, numvalues);
		/* Get the wall clock time at finish */
		clock_gettime(CLOCK_MONOTONIC, &finish);

		/* Calculate time elapsed */
		elapsed = (finish.tv_sec - start.tv_sec);
		elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

		/* Print the time elapsed (in seconds) */

		printf("\n\nTiempo: %lf\n\n\n", elapsed);

        printf("\n");
		printf("Tiempo quicksort: %lf\n", elapsed);


		printf("\nTiempo quicksort: %lf\n\n", elapsed);

        // 
        //quicksort(readbuf, 0, numvalues);
        /* Print out the values obtained from datagen */
        /*
        printf("S%d:\n", i);
        for (UINT *pv = readbuf; pv < readbuf + numvalues; pv++) {
            printf("%u\n", *pv);
        }*/

        free(readbuf);
        printf("Fin de la ejecucion numero %d.\n\n", i);
    }

    /* Issue the END command to datagen */
    int rc = strlen(DATAGEN_END_CMD);
    if (write(fd, DATAGEN_END_CMD, strlen(DATAGEN_END_CMD)) != rc) {
        if (rc > 0) fprintf(stderr, "[quicksort] partial write.\n");
        else {
            perror("[quicksort] write error.\n");
            close(fd);
            exit(-1);
        }
    }

    close(fd);
    exit(0);
}