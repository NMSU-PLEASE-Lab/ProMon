#include <cstdlib>
#include <limits.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_flowGraph.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "tinyxml.h"
#include "predefs.h"
#include "parser.h"
#include "util.h"

#include <sys/types.h>
#include <unistd.h>
#include <csignal>

using namespace std;

/*
 * This enum defines if injector is running the application.
 * create:      injector runs the application to perform dynamic instrumentation.
 * attach:      injector attaches itself to a running application to perform dynamic instrumentation.
 * open:        injector opens the binary file to perform static instrumentation.
 */
typedef enum
{
   create, attach, open
} accessType_t;

class Injector
{
   public:
   Injector(){itself=this;};
   void shutdown( int signum );
   void instrument(int argc, char* argv[]);
   static void shutdown_static( int signum );
   static void instrument_static(int argc, char* argv[]);


   private:
   void startInstrumentation(BPatch_addressSpace *app, string strArgv, vector<instRecord> arrangedrecs);
   BPatch_addressSpace *loadApplicationImage(accessType_t accessType,
      const char *name, int pid, const char *argv[]);
   void finishInstrumenting(BPatch_addressSpace *app, const char *newName, 
      const char *dynamicType);
   void defineProcLimiterEnvironmentVariable();
   void parseApplicationParameter(int argc, char* argv[]);

   static Injector *itself;   

   BPatch bpatch;
   /*
    * To run the application, we need to have arguments in the form array of strings
    */
   const char *progArgv[100];

   /*
    * first arg is the name of the application
    * strArgv: Contains all the parameters of the application including the application name.
    */
   string strArgv;
  
   /* shift is used to parse the argument */
   int shift;
};
