
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * The code can be compiled in parallel (mpi) or serial.
 */
#ifdef MT_WITH_MPI
#include <mpi.h>
#endif

short global0 = 10;
int global1 = 100;
float global2 = 12.21;
long global3 = 123;
double global4 = 123.123;


int doSimIteration(int iterNum)
{
   printf("testapp begin iteration %d...\n",iterNum);
   sleep(2);
   printf("testapp  end  iteration %d...\n",iterNum);

   return iterNum % 4;
}

void mainBody(int numIters, int argc, char *argv[])
{
   printf("the proces id is : %d \n", getpid()); 
   int i;

   //
   // Probe module was failing to find the clock_gettime symbol because it is
   // in librt.so and was not being loaded (and LD_PRELOAD would refuse to?)
   // - this is a temporary fix and a real fix needs to be done
   //
   struct timespec time1;
   clock_gettime(CLOCK_REALTIME, &time1);

   // Print args to see if we are getting them properly
   for (i=0; i < argc; i++) {
      printf("testapp arg[%d] = (%s)\n",i,argv[i]);
   }

   // Simulate a slow simulation loop where each step is in
   // a function
   for (i=0; i < numIters; i++)
      doSimIteration(i);
}


int main(int argc, char *argv[])
{
#ifdef MT_WITH_MPI
   MPI_Init(&argc, &argv); /*START MPI */
#endif

   /*
    * We kept the entire body of the main in this function.
    * As a test, if we instrument the "main" at the begin and end, then when the
    * application runs in MPI mode, instrumentation occurs prior to calling MPI_Init.
    * This will cause runtime error.
    */
   mainBody(10, argc, argv);

#ifdef MT_WITH_MPI
   MPI_Finalize();  /* EXIT MPI */
#endif

   return 0;
}

