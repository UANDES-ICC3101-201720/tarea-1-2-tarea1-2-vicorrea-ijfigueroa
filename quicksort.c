#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "const.h"
#include "util.h"

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
int parallel_quicksort(UINT* A, int lo, int hi) {
    return 0;
}

int main(int argc, char** argv) {
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
            if(t < 3 || t > 9){
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
        printf("E%d: ",i);
        for (UINT *pv = readbuf; pv < readbuf + numvalues; pv++) {
            printf("%u, ", *pv);
        }
        // Quicksorting
		struct timespec start, finish;
		double elapsed = 0;

		/* Get the wall clock time at start */
		clock_gettime(CLOCK_MONOTONIC, &start);

		quicksort(readbuf, 0, numvalues);

		/* Get the wall clock time at finish */
		clock_gettime(CLOCK_MONOTONIC, &finish);

		/* Calculate time elapsed */
		elapsed = (finish.tv_sec - start.tv_sec);
		elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

		/* Print the time elapsed (in seconds) */
		printf("%lf\n", elapsed);
        // 
        quicksort(readbuf, 0, numvalues);
        /* Print out the values obtained from datagen */
        printf("\nS%d: ",i);
        for (UINT *pv = readbuf; pv < readbuf + numvalues; pv++) {
            printf("%u, ", *pv);
        }

        free(readbuf);

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