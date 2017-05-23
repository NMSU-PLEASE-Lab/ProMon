//
//  Usage: instrumentor <app-binary> [app-arguments]
//
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

BPatch bpatch;

void instrument(const char *name, const char *argv[])
{
   bool status;
   std::vector<BPatch_function *> functions;
   std::vector<BPatch_point *> *points;
   BPatchSnippetHandle *sh;

   BPatch_addressSpace *appHandle = NULL;
   appHandle = bpatch.processCreate(name, argv);
   //appHandle = bpatch.processAttach(name, pid);
   //appHandle = bpatch.openBinary(name);
   BPatch_image *appImage = appHandle->getImage();

   status = appHandle->loadLibrary("./libprobe.so");
   status = appImage->findFunction("myProbe", functions);
   status = appImage->findFunction("doSimIteration", functions);

   BPatch_Vector<BPatch_snippet *> args;
   //args.push_back("arg");
   BPatch_funcCallExpr probeFunction(*functions[0], args);
   points = NULL; //initialize to avoid warning.
   points = functions[1]->findPoint(BPatch_entry);
   sh = appHandle->insertSnippet(probeFunction, *points);
   points = functions[1]->findPoint(BPatch_exit);
   sh = appHandle->insertSnippet(probeFunction, *points);

   BPatch_process *appProc = dynamic_cast<BPatch_process *>(appHandle);
   appProc->continueExecution();
   while (!appProc->isTerminated())
   {
      bpatch.waitForStatusChange();
   }
/*
   //BPatch_binaryEdit *appBin = dynamic_cast<BPatch_binaryEdit *>(app);
   if (appBin)
   {
      appBin->writeFile(newName);
      if (DEBUG)
         cout << "I did some binary writes\n";
   }
*/
}


int main(int argc, const char* argv[])
{
   if ((argc-1) < 1)
   {
      cerr << "Usage: " << argv[0]
            << " <app-binary> [app-argument1 app-argument2 app-argument3 ...]\n";
      return 1;
   }

   const char *progArgv[100];
   int i;
   for(i=0; i < argc; i++)
   {
      progArgv[i] = argv[i+2];
   }
   progArgv[i]=NULL;

   instrument(argv[1], progArgv);

   return 0;
}

