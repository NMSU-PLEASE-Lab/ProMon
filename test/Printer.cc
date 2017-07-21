/*
 * This program is used to print the loops in binary executable
 * Argument can be function name or empty
 * If function name passed then loops on that function will be printed
 * Otherwise every function with corresponding loops will be printed
 */

#include <stdio.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"
#include "CodeObject.h"

#define DYNINST_API_RT_PATH "/home/ujjwal/Build/DyninstAPI-9.3.2/lib/libdyninstAPI_RT.so"
using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
BPatch bpatch;


/*
 * Prints the function name and loops in the function
 */
void printFunctionsAndLoops(BPatch_function *function)
{

    vector<BPatch_basicBlockLoop *> loops;
    BPatch_flowGraph *fg = function->getCFG();
    cout<<endl;
    cout<<"------------------------------------------------------------------"<<endl;
    cout<<"Loops for function: "<<function->getName()<<endl;
    cout<<"------------------------------------------------------------------"<<endl;
    fg->printLoops();

}

int main(int argc, char *argv[])
{
    BPatch_addressSpace *app;

    app = bpatch.openBinary(argv[1],true);
    BPatch_image *appImage = app->getImage();
    app->loadLibrary(DYNINST_API_RT_PATH);
    if(argc>2){
        std::vector<BPatch_function *> functions;
        appImage->findFunction(argv[2],functions);
        for(int i = 0; i<functions.size();i++)
            printFunctionsAndLoops(functions[i]);
        return 0;
    }

    SymtabCodeSource *sts;
    CodeObject *co;

    sts = new SymtabCodeSource(argv[1]);
    co = new CodeObject(sts);

    co->parse();
    const CodeObject::funclist &all = co->funcs();
    set<Function *>::iterator funcIter;
    funcIter = all.begin();
    for(;funcIter != all.end(); ++funcIter){
        Function *f = *funcIter;
        std::vector<BPatch_function *> functions;
        appImage->findFunction(f->name().c_str(),functions);
        for(int i = 0; i<functions.size();i++)
            printFunctionsAndLoops(functions[0]);
    }

    return 0;
}
