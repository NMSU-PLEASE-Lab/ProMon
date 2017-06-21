#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

/*
 * The code can be compiled in parallel (mpi) or serial.
 */
#ifdef MT_WITH_MPI
#include <mpi.h>
#endif
#define NUM_OF_ITERATIONS 2
#define TEST_NUMBER 2
#define SLEEP_TIME 10


short global0 = 10;
int global1 = 100;
float global2 = 12.21;
long global3 = 123;
double global4 = 123.123;


int doSimIteration(int numIters, int testNumber)
{
    printf("BEGIN: Function doSimIteration\n");
    if (testNumber < 30) {
        printf("BEGIN: Loop 1 (doSimIteration)\n");
        for (int i = 0; i <= numIters / 3; i++) {
            printf("BEGIN: Loop 1.1 (doSimIteration)\n");
            for (int j = 0; j < numIters; ++j) {
            }
            printf("END: Loop 1.1 (doSimIteration)\n");
        }
        printf("END: Loop 1 (doSimIteration)\n");
    } else {
        printf("BEGIN: Loop 2 (doSimIteration)\n");
        for (int i = 0; i <= numIters / 4; i++) {
            printf("BEGIN: Loop 2.1 (doSimIteration)\n");
            for (int j = 0; j < numIters; ++j) {
            }
            printf("END: Loop 2.1 (doSimIteration)\n");
        }
        printf("END: Loop 2 (doSimIteration)\n");
    }
    usleep(SLEEP_TIME*1000); //sleep for SLEEP_TIME (milliseconds)
    printf("END: Function doSimIteration\n");
    return numIters % 4;

}

void mainBody(int argc, char *argv[])
{
    printf("BEGIN: Function mainBody\n");
    printf("Process id is : %d \n", getpid());
    int i;
    int numIters = NUM_OF_ITERATIONS;
    int testNumber = TEST_NUMBER;
    printf("Global 1:%d",global1);

    if (argc > 1) {
        numIters = strtoumax(argv[1], NULL, 10);
    }
    if (argc > 2) {
        testNumber = strtoumax(argv[2], NULL, 10);
    }


    // Print args to see if we are getting them properly
    printf("BEGIN: Loop 1 (mainBody)\n");
    for (i = 0; i < argc; i++) {
        printf("Argument[%d] = %s\n", i, argv[i]);
    }
    printf("END: Loop 1 (mainBody)\n");

    printf("BEGIN: Loop 2 (mainBody)\n");
    for (i = 0; i < numIters; i++) {
        printf("BEGIN: Loop 2.1 (mainBody)\n");
        for (int j = 0; j < numIters; j++) {
            printf("BEGIN: Loop 2.1.1 (mainBody)\n");
            for (int k = 0; k < numIters; k++) {

            }
            printf("END: Loop 2.1.1 (mainBody)\n");
        }
        printf("END: Loop 2.1 (mainBody)\n");

        printf("BEGIN: Loop 2.2 (mainBody)\n");
        for (int j = 0; j < numIters; j++) {
            printf("BEGIN: Loop 2.2.1 (mainBody)\n");
            for (int k = 0; k < numIters; k++) {

            }
            printf("END: Loop 2.2.1 (mainBody)\n");
        }
        printf("END: Loop 2.2 (mainBody)\n");
    }
    printf("END: Loop 2 (mainBody)\n");

    printf("BEGIN: Loop 3 (mainBody) \n");
    for (i = 0; i < numIters; i++) {
        printf("BEGIN: Loop 3.1 (mainBody) \n");
        for (int j = 0; j < numIters; ++j) {
            doSimIteration(numIters, testNumber);
        }
        printf("END: Loop 3.1 (mainBody) \n");
    }
    printf("END: Loop 3 (mainBody) \n");

    // Simulate a loop with conditional statements
    if (testNumber < 50) {
        printf("BEGIN: Loop 4 (mainBody) \n");
        for (i = 0; i < numIters; i++) {
            printf("BEGIN: Loop 4.1 (mainBody) \n");
            for (int j = 0; j < numIters; ++j) {
                doSimIteration(numIters, testNumber);
            }
            printf("BEGIN: Loop 4.1 (mainBody) \n");
        }
        printf("END: Loop 4 (mainBody) \n");

    } else {
        printf("BEGIN: Loop 5 (mainBody) \n");
        for (i = 0; i <= numIters / 2; i++) {
            printf("BEGIN: Loop 5.1 (mainBody) \n");
            for (int j = 0; j < numIters; ++j) {
                doSimIteration(numIters, testNumber);
            }
            printf("END: Loop 5.1 (mainBody) \n");
        }
        printf("BEGIN: Loop 5 (mainBody) \n");

    }
    printf("END: Function mainBody\n");


}

void printFunctionForTesting(const char *args,void* varAdd)
{
    printf("%s",args);
}
void printFunctionForTesting2(void* varAdd)
{
    int *temp = static_cast<int*> (varAdd);
}

int main(int argc, char *argv[])
{
    printf("BEGIN: Function main\n");

#ifdef MT_WITH_MPI
    MPI_Init(&argc, &argv); /*START MPI */
#endif

    /*
     * We kept the entire body of the main in this function.
     * As a test, if we instrument the "main" at the begin and end, then when the
     * application runs in MPI mode, instrumentation occurs prior to calling MPI_Init.
     * This will cause runtime error.
     */
    mainBody(argc, argv);

#ifdef MT_WITH_MPI
    MPI_Finalize();  /* EXIT MPI */
#endif

    printf("END: Function main\n");
    return 0;
}

