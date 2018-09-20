#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

int main(int argc, char** argv) {
	struct timespec start, finish;
	double elapsed = 0;

	/* Get the wall clock time at start */
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* Burn CPU time */
	for (int i = 0; i<INT_MAX/2; i++);

	/* Get the wall clock time at finish */
	clock_gettime(CLOCK_MONOTONIC, &finish);

	/* Calculate time elapsed */
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

	/* Print the time elapsed (in seconds) */
	printf("%lf\n", elapsed);
	return 0;
}
