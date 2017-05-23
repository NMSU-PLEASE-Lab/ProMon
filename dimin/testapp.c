
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

/*
 * FUTURE:The code can be compiled in parallel (mpi) or serial.
#ifdef MT_WITH_MPI
#include <mpi.h>
#endif
 */

//
// The test injector just puts a probe at the beginning
// and end of this function
//
int doSimIteration(int iterNum)
{
   printf("testapp begin iteration %d...\n",iterNum);
   sleep(2);
   printf("testapp  end  iteration %d...\n",iterNum);
   return iterNum % 4;
}

void mainBody(int argc, char *argv[])
{

   int i;

   // NOT RELEVANT NOW:
   // Probe module was failing to find the clock_gettime symbol because it is
   // in librt.so and was not being loaded (and LD_PRELOAD would refuse to?)
   // - this is a temporary fix and a real fix needs to be done
   //
/*
   struct timespec time1;
   clock_gettime(CLOCK_REALTIME, &time1);
*/

   // Print args to see if we are getting them properly
   for (i=0; i < argc; i++) {
      printf("testapp arg[%d] = (%s)\n",i,argv[i]);
   }

   // Simulate a slow simulation loop where each step is in
   // a function
   for (i=0; i < 5; i++)
      doSimIteration(i);
}


int main(int argc, char *argv[])
{
/*
#ifdef MT_WITH_MPI
   MPI_Init(&argc, &argv);
#endif
*/

   /*
    * We kept the entire body of the main in this function.
    * As a test, if we instrument the "main" at the begin and end, then when the
    * application runs in MPI mode, instrumentation occurs prior to calling MPI_Init.
    * This will cause runtime error.
    */
   mainBody(argc, argv);

/*
#ifdef MT_WITH_MPI
   MPI_Finalize();
#endif
*/

   return 0;
}

