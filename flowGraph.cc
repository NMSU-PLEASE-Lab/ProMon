//
//  Usage: instrumentor <monspec-xml> <app-binary> [app-arguments]
//
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
#include <signal.h>

using namespace std;

/*
 * Make sure log files are closed after terminating.
 */
void signalHandler( int signum )
{
   printf("ProMon Injector: Interrupt signal (%d) received.", signum);
   ProMon_logger_close(); 
   exit(signum);
}


/*
 * This enum defines if injector is running the application.
 * create:      injector runs the application to perform dynamic instrumentation.
 * attach:      injector attaches itself to a running application to perform dynamic instrumentation.
 * open:        injector opens the binary file to perform static instrumentation.
 */
BPatch bpatch;
typedef enum
{
   create, attach, open
} accessType_t;

/*
 * First phase of of instrumentation.
 * By this function, injector instruments in one of the three modes (create, attach and open).
 */
BPatch_addressSpace *openBinary(const char *name)
{
   BPatch_addressSpace *handle = NULL;
   handle = bpatch.openBinary(name);
   return handle;
}

/*
 * The second phase of instrumentation.
 * injector, reads the instrumentation XML file, loads the ProMon probe library and perform the instrumentation
 * based on the input data.
 */
void binaryAnalysis(BPatch_addressSpace *app)
{
   string line;

   bool load = false;
   std::vector<BPatch_function *> functions;
   std::vector<BPatch_function *> probeFunctions;
   BPatch_image *appImage = app->getImage();

//////////////////////////////////////////////////////////////////////////////////////
/////////instrument the defined function in xml file with probe functions/////////////

   /*
    * all records are provided to injector by getRecord() function.
    *
    */
   vector<instRecord> arrangedrecs = getRecord();
   set<string> funcs; 
   for (vector<instRecord>::iterator it = arrangedrecs.begin();
         it != arrangedrecs.end(); ++it)
   {
      funcs.insert(it->function);
   }
   

   for (set<string>::iterator it = funcs.begin();
         it != funcs.end(); ++it)
   {
      
      /*
       * Find the function in the running application.
       * The details on how instrument the function is defined in the input XML data
       * Last parameter, ensures that regular expression is used.
       */
      functions.clear();
      load = appImage->findFunction(it->c_str(), functions, true, true, true);

      if (load)
      {
         ProMon_logger(PROMON_INFO, "--->found. total: %d ", functions.size());
         //cout<<functions[i]->getName()<<endl;
      }
      else
      {
         ProMon_logger(PROMON_INFO, "--->NOT found!!");
      }

      for(unsigned int f=0; f<functions.size(); f++)
      {
         std::set <BPatch_basicBlock*> basicblocks;
         BPatch_flowGraph *fg = functions[f]->getCFG();
         fg->getAllBasicBlocks(basicblocks);
         ProMon_logger(PROMON_INFO, "%s has  %d BasicBlocks:", functions[f]->getName().c_str(), (int)basicblocks.size());
         
         int count = 0;
         std::set<BPatch_basicBlock *>::iterator itr;
         for(itr = basicblocks.begin(); itr != basicblocks.end(); itr++)
         {
            BPatch_basicBlock *bb = *itr; 
            ProMon_logger(PROMON_INFO, "#%d Starts: %llu  Ends: %llu ", count, bb->getStartAddress(), bb->getEndAddress()); 
            count++;
         }
  
         std::vector <BPatch_basicBlockLoop*> loops;
         fg->getLoops(loops);
         ProMon_logger(PROMON_INFO, "%s has %d Loops:", functions[f]->getName().c_str(), (int)loops.size());
         
         count = 0;
         std::vector<BPatch_basicBlockLoop *>::iterator itrLoops;
         for(itrLoops = loops.begin(); itrLoops != loops.end(); itrLoops++)
         {
            BPatch_basicBlockLoop *bbloop = *itrLoops; 
            BPatch_basicBlock *bb = bbloop->getLoopHead();
            ProMon_logger(PROMON_INFO, "#%d Starts: %llu Ends: %llu ", count, bb->getStartAddress(), bb->getEndAddress()); 
            count++;
         }

         //std::vector <BPatch_basicBlockLoop*> loops2;
         //fg->getOuterLoops(loops2);
         //ProMon_logger(PROMON_INFO, "There are %d OuterLoops:", (int)loops2.size());
         //count = 0;
         //for(itrLoops = loops2.begin(); itrLoops != loops2.end(); itrLoops++)
         //{
         //   BPatch_basicBlockLoop *bbloop = *itrLoops; 
         //   BPatch_basicBlock *bb = bbloop->getLoopHead();
         //   ProMon_logger(PROMON_INFO, "#%d Starts: %llu Ends: %llu ", count, bb->getStartAddress(), bb->getEndAddress()); 
         //   count++;
         //}
      }
   }
}

int main(int argc, char* argv[])
{
   /*
    * Initializing the log file
    */
   ProMon_logger_init("LOG_FlowGraph");

   /*
    * Make sure no file kept open after terminating
    */
   signal(SIGINT, signalHandler);


   if ((argc - 1) < 2)
   {
      ProMon_logger(PROMON_ERROR,
                    "Flow Graph usage: %s <monspec-xml> <app-binary> \n", argv[0]);
      return 1;
   }


   /*
    * Read the injector input xml file.
    * Files are all in pminput directory.
    */
   load(argv[1]);


   /*
    * The first phase of instrumentation.
    * startInstrumentation runs the application first.
    * either it is a binary instrumentation or dynamic instrumention.
    */
   BPatch_addressSpace *app;
   app = openBinary(argv[2]);

   /*
    * The second phase of instrumentation.
    * This part, instrument the application based on the instrumentation instruction input (previously called MonToolLang)
    */
   binaryAnalysis(app);

   ProMon_logger_close();
   return 0;
}

