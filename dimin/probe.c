#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*
 * The code can be compiled in parallel (mpi) or serial.
#ifdef MT_WITH_MPI
#include <mpi.h>
#endif
 */

int myProbe(const char *tag)
{
   /*
    * Currently we are checking rank 0.
    * This part will be changed to send record from all processes.
#ifdef MT_WITH_MPI
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
   rank = 0;
#endif
    */
   fprintf(stderr,"my probe: (%s)\n", tag);
   return 0;
}

