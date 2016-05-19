#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/***** Globals ******/

int NUM_INPUT = 3;
int N_POS = 1;
int T_POS = 2;

/********************/


/****** Function declarations ******/
// Helper functions
int get_num_digits(int x); /* returns the number of digits in an integer */

// Input/Output functions
void get_input(int argc, char *argv[], int *N, int *t); /* place command-line args into N and t */
int write_output(int N, int *nums); /* write to the N.txt file with formatted info for prime numbers */

// Algorithm functions
void generate_primes(int N, int t, int *nums); /* generate prime numbers based on N and t */
/***********************************/



/*****************************************************************************/
/* returns the number of digits in an integer */
int get_num_digits(int x)
{
	int digits = 0;

	while (x)
	{
		x /= 10;
		++digits;
	}

	return digits;
}


/*****************************************************************************/
/* read inputs from the command-line */
void get_input(int argc, char *argv[], int *N, int *t)
{
	if (argc != NUM_INPUT)
	{
		printf("Invalid number of inputs.\n");
		exit(1);
	}

	*N = atoi(argv[N_POS]);
	*t = atoi(argv[T_POS]);

	if (*N < 2)
	{
		printf("Value for N must be at least 2.\n");
		exit(1);
	}

	if (*t < 1)
	{
		printf("Number of threads, t, cannot be less than 1.\nt will be set to 1.\n");
		*t = 1;
	}
}


/*****************************************************************************/
/* generates prime numbers based on numbers from 2 to N using t threads
 *	1. Generate all #s from 2 to N
 *	2. First # is 2, so remove all #s that are multiples of 2 (i.e. 4, 6, 8, ..., N).
 	   Do not remove 2 itself.
 *	3. Following # is 3, so remove all multiples of 3 that have not been removed
 	   from the previous step. That will be: 9, 15, ..., till you reach N.
 *	4. Next # that has not been crossed so far is 5. So, remove all multiples
 	   of 5 that have not been crossed before, till you reach N.
 *	5. Continue like this till floor((N+1)/2).
 *	6. The remaining #s are the prime numbers.
 */
void generate_primes(int N, int t, int *primes)
{
	// int nums[N+1], local_nums[t][N+1];

// #pragma omp for 
	// store the numbers from 2, ..., N in the associated array position: 2->2, ..., N->N
	// for (i = 2; i <= N; ++i)
	// {
	// 	nums[i] = i;
	// }

	int nt = omp_get_num_threads();
	int master_j;
	int local_primes[nt][(N-1)/nt];

#pragma omp parallel num_threads(t)
	{
		int tid = omp_get_thread_num();
		// int nt = omp_get_num_threads();

		int i, j, k = 0;
		int curr_num;


		// assign each thread a portion of the numbers from 2, ..., N
		#pragma omp for nowait schedule(static,1)
		for (i = 2; i <= N; ++i)
		{
			local_primes[tid][j++] = i;
		}
		// NO IMPLICIT BARRIER

		if (tid == 0)
		{
			master_j = j;
		}

		// loop over the elements in local_primes
		for (i = 0; i < j; ++i)
		{
			printf("Here\n");

			curr_num = local_primes[tid][i];
			// check each number in local_primes to see if it is a multiple
			for (k = 2; k <= ((N+1)/2); ++k)
			{
				// current number is a multiple
				if ((curr_num % k) == 0)
				{
					// current number should only be removed if it is a multiple of a smaller number
					if (curr_num != k)
					{
						local_primes[tid][i] = -1;
						break;
					}
				}
				// current number cannot be a multiple of a larger number
				else if (curr_num <= k)
				{
					break;
				}
			}
		}

		// // loop over the elements in local_primes
		// for (i = 0; i < j; ++i)
		// {
		// 	curr_num = local_primes[i];
		// 	// if the current number isn't 0, it is a prime number
		// 	if (curr_num != 0)
		// 	{
		// 		// store it in the global primes array
		// 		primes[curr_num] = curr_num;
		// 	}
		// }

	}

printf("Here\n");

	int i, k, curr_num;

	for (i = 0; i < nt; ++i)
	{
		for (k = 0; k < master_j; ++k)
		{
			curr_num = local_primes[i][k];
			if (curr_num != -1)
			{
				primes[curr_num] = curr_num;
			}
		}
	}


}


/*****************************************************************************/
/* write to the N.txt file with formatted info for prime numbers.
 *	- one prime per line
 *	- each line has the format: a, b, c
 *		- a: the rank of the number (1 means the first prime)
 *		- b: the number itself
 *		- c: the interval from previous prime number (i.e. the current prime - previous prime)
 *	- assume the first line of the file to be: 1, 2, 0
 *	- the second line will then be: 2, 3, 1
 *	- and the third: 3, 5, 2
 *	- and so on...
 */
int write_output(int N, int *nums)
{
	int i;

	// create the output file name as N.txt
	char filename[get_num_digits(N) + 4 + 1]; // "N" + ".txt" + "\0" -> N digits + 4 + 1
	sprintf(filename, "%d.txt", N);

	// open the file in write mode (creates a new file if one doesn't exist)
	FILE *fp = fopen(filename, "w");

	// write to the file in the formatted specified
	int rank = 1;		// the first prime number is always ranked 1
	int prev_num = 2;	// the first prime number is always 2
	int curr_num = 2;	// the first (and current) prime number is always 2

	// loop over 2, ..., N to find the prime numbers
	for (i = 2; i <= N; ++i)
	{
		// if the index does not store a 0, then it is a prime number
		if (nums[i] != 0)
		{
			curr_num = i; // the prime number
			// write the formatted output to the file and post-increment rank
			fprintf(fp, "%d, %d, %d\n", rank++, curr_num, (curr_num - prev_num));
			prev_num = curr_num; // next prime number needs to know the interval
		}
	}

	// close the file
	fclose(fp);

	return 0; // return 0 on success

}

int main(int argc, char *argv[])
{
	/*************************************************************************/

	/* Read input from command-line */
	
	int N, t;

	get_input(argc, argv, &N, &t);



	/* Generate the prime numbers (and do timing) */

	double tstart = 0.0, tend = 0.0, ttaken;

	tstart = omp_get_wtime();
	// START TIMING

	int *primes = (int *) calloc(sizeof(int), (N+1)); // allocate array to store 2, ..., N
	// int *primes = (int *) calloc((N+1) * sizeof(int)); // allocate array to store 2, ..., N

	generate_primes(N, t, primes);

	// STOP TIMING
	tend = omp_get_wtime();

	ttaken = tend - tstart;

	printf("Time taken for the main part: %f\n", ttaken);



	/* Write the output file and exit */

	int code = write_output(N, primes);
	if (code != 0) exit(1); // exit with error

	/*************************************************************************/

	free(primes);

	exit(0);
}




// #pragma omp parallel num_threads(t) shared(N, t, i, nums) private(j)
// 	{
// 		int tid = omp_get_thread_num();
// 		int index = 0;

// 		while (i <= ((N+1)/2))
// 		{
// 			if (nums[i] != 0) 
// 			{
// 				#pragma omp single nowait
// 				primes[primes_index++] = i 

// 				#pragma omp for
// 				for (j = i * i; j <= N; j += i)
// 				{
// 					local_nums[tid][index++] = j;
// 				}

// 				for (j = 0; j < index; ++j)
// 				{
// 					#pragma omp critical
// 					nums[local_nums[tid][j]] *= 0;
// 				}

// 				index = 0;
// 			}



// 			#pragma omp single
// 			{

// 				++i;
// 			}
// 		}
// 	}







// 	// create a 2-D array: each row represents non-prime numbers found by one thread
// 	// int local_nums[][] = int[t-1][(N/t) + 1];
// 	int local_nums[t][N+1];


// // #pragma omp for 
// 	// store the numbers from 2, ..., N in the associated array position: 2->2, ..., N->N
// 	for (i = 2; i <= N; ++i)
// 	{
// 		nums[i] = i;
// 	}

// 	i = 2;



// #pragma omp parallel num_threads(t) shared(N, t, i, nums) private(j)
// 	{
// 		int tid = omp_get_thread_num();
// 		int index = 0;

// 		while (i <= ((N+1)/2))
// 		{
// 			if (nums[i] != 0)
// 			{
// 				#pragma omp for
// 				for (j = i + i; j <= N; j += i)
// 				{
// 					local_nums[tid][index++] = j;
// 				}

// 				for (j = 0; j < index; ++j)
// 				{
// 					#pragma omp critical
// 					nums[local_nums[tid][j]] *= 0;
// 				}

// 				index = 0;
// 			}



// 			#pragma omp single
// 			{
// 				++i;
// 			}
// 		}
// 	}





// 		for (i = 2; i <= ((N+1)/2); ++i)
// 		{
// 			printf("tid: %d - i: %d\n", tid, i);
// 			if (nums[i] != 0)
// 			{
// #pragma omp for
// 				for (j = i + i; j <= N; j += i)
// 				{
// #pragma omp critical
// 					nums[j] *= 0;
// 				}
// 			}
// 		}
// 	}




// #pragma omp parallel num_threads(t) firstprivate(N, t)
// 	{
// 		int tid = omp_get_thread_num();
// 		int index = 0;

// 		for (i = 2; i <= ((N+1)/2); ++i)
// 		{
// #pragma omp for
// 				for (j = i + i; j <= N; j += i)
// 				{
// 					printf("thread: %d, index: %d -> j: %d\n", tid, index, j);

// 					// local_nums[tid][index++] = j;
// 				}
// 				// #pragma omp critical
// 				// printf("thread: %d -> index: %d\n", tid, index);
// 				// local_nums[tid][index] = 0;
// 				// index = 0;

// // #pragma omp for
// // 			{

// // 			}
// 		}
// 	}




// #pragma omp parallel num_threads(t) firstprivate(N, t)
// 	{
// 		int tid = omp_get_thread_num();
// 		// int start = 0, end = 0;
// 		int index = 0;

// 		// loop over 2, ..., floor((N+1)/2) to find possible prime numbers
// 		for (i = 2; i <= ((N+1)/2); ++i)
// 		{
// 			// if the index does not store a 0, then it is a prime number (since it has not been removed)
// 			if (nums[i] != 0)
// 			{
// #pragma omp for
// 				{
// 					// i is prime: remove multiples of i by setting its array index to 0
// 					for (j = i + i; j <= N; j += i)
// 					{
// 						if (nums[j] != 0)
// 						{
// 							local_nums[tid][index++] = j;
// 						}
// 					}

// 					// local_nums[tid][index+1] = 0;
// 				}

// #pragma omp for
// 				for (j = i + i; j <= N;  )


// 			}
// 		}
// 	}



// #pragma omp parallel num_threads(t) firstprivate(N, t, j)
// 	{
// 		int thread_num = omp_get_thread_num();

// 		// loop over 2, ..., floor((N+1)/2) to find possible prime numbers
// 		for (i = 2; i <= ((N+1)/2); ++i)
// 		{
// 			if (nums[i] != 0)
// 			{

// #pragma omp for
// 				for (j = i + i; j <= N; j += i)
// 				{
// 					if (local_nums[thread_num][j] == 0)
// 					{
// 						local_nums[thread_num][j] = 1;
// 					}
// 				}

// #pragma omp for
// 				for (j = i; j <= N; ++j)
// 				{
// 					if (nums[j] != 0)
// 					{
// 						if (local_nums[thread_num][j] == 0)
// 						{
// 							nums[j] = 0;
// 						}
// 					}
// 				}

// 			}
// 		}
// 	}


// #pragma omp parallel num_threads(t) shared(N, t, nums) private(j)
// 	{
// 		int tid = omp_get_thread_num();

// #pragma omp for schedule(static, 1)
// 		for (i = 2; i <= ((N+1)/2); ++i)
// 		{
// 			if (nums[i] != 0)
// 			{
// 				for (j = i + i; j <= N; j += i)
// 				{
// 					nums[j] = 0;
// 				}
// 			}
// 		}


// 	}



// }