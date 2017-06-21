#include <stdio.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"

using namespace Dyninst;
using namespace std;


// Find a point at which to insert instrumentation
std::vector<BPatch_point *> *findPoint(BPatch_addressSpace *app,
                                       const char *name,
                                       BPatch_procedureLocation loc,int loopNumber)
{
    std::vector<BPatch_function *> functions;
    std::vector<BPatch_point *> *points;
    BPatch_image *appImage = app->getImage();
    appImage->findFunction(name, functions);
    if (functions.size() == 0) {
        fprintf(stderr, "No function %s\n", name);
    } else if (functions.size() > 1) {
        fprintf(stderr, "More than one %s; using the first one\n", name);
    }
    //points = functions[0]->find(loc);

    vector<BPatch_basicBlockLoop *> loops;
    BPatch_flowGraph *fg = functions[0]->getCFG();

    fg->printLoops();
    fg->getLoops(loops);

    BPatch_basicBlockLoop *loop = fg->findLoop("loop_1") ;
//    vector<BPatch_basicBlock *> entries;
//    loop->getLoopEntries(entries);
//
//    vector<BPatch_basicBlock *>::iterator block_iter;
//    block_iter = entries.begin();
//
//    for (int i = 0; block_iter!=entries.end(); i++)
//    {
//        block_iter++;
//    }
//    BPatch_basicBlock *block = *block_iter;
//    points->insert(*block->findEntryPoint())
//
    if (!loop) {
        fprintf(stderr, "loop doesn't exit\n");
        return NULL;
    }
    points = fg->findLoopInstPoints(BPatch_locLoopEndIter, loop);
    if (!points) {
        fprintf(stderr, "No load/store points found\n");
    }
    return points;
}

int main(int argc, char *argv[])
{
    // process control
    BPatch_addressSpace *app;
//    const char* progArgv[] = {"lulesh2.0", "-h", NULL};
//    app = bpatch.openBinary("testapp",true);
    app = bpatch.openBinary("/home/ujjwal/Source/applications/lulesh2.0.3/lulesh2.0",true);
    BPatch_image *appImage = app->getImage();
    app->loadLibrary("/home/ujjwal/Build/DyninstAPI-9.3.2/lib/libdyninstAPI_RT.so");
    // Find the entry point for function InterestingProcedure
    const char *interestingFuncName = "InitStressTermsForElems";
    std::vector<BPatch_point *> *entryPoint =
        findPoint(app, interestingFuncName, BPatch_entry,1);
    if (!entryPoint || entryPoint->size() == 0) {
        fprintf(stderr, "No entry points for %s\n", interestingFuncName);
        exit(1);
    }

//    BPatch_variableExpr *variable = appImage->findVariable("x");

    // Create the printf function call snippet
    std::vector<BPatch_snippet*> printfArgs;
//    BPatch_snippet* fmt = new BPatch_constExpr(variable->getBaseAddr());
    BPatch_snippet* fmt = new BPatch_constExpr("Start of loop 1.1");
    printfArgs.push_back(fmt);
    printfArgs.push_back(fmt);
//
    // Find the printf function
    std::vector<BPatch_function*> printfFuncs;
    appImage->findFunction("printFunctionForTesting", printfFuncs);

    if (printfFuncs.size() == 0) {
        fprintf(stderr, "Could not find printf\n");
        return 0;
    }

    // Construct a function call snippet
    BPatch_funcCallExpr printfCall(*(printfFuncs[0]), printfArgs);


    if (!app->insertSnippet(printfCall, *entryPoint,BPatch_callBefore)) {
        fprintf(stderr, "insertSnippet failed\n");
        return 0;
    }
    else {
//        BPatch_process* appProc = dynamic_cast<BPatch_process*>(app);
        BPatch_binaryEdit* appBin = dynamic_cast<BPatch_binaryEdit*>(app);

//        BPatch_binaryEdit *appBin = dynamic_cast<BPatch_binaryEdit *>(app);
        appBin->writeFile("lulesh.instrumented");
    }
    return 0;
}
