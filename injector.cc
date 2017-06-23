#include "injector.h"

using namespace std;
//============= static properties and methods of Injector ==============

/* static properties needs to be initialized out of the class */
/* keep a pointer to itself for static calls */
Injector *Injector::itself = NULL;

/* The static shutdown method */
void Injector::shutdown_static(int num)
{
    if (itself != NULL)
        itself->shutdown(num);
}

/* The static instrument  method */
void Injector::instrument_static(int argc, char *argv[])
{
    if (itself != NULL)
        itself->instrument_static(argc, argv);
}

//============= regular  methods of Injector =========================

/*
 * Make sure log files are closed after terminating.
 */
void Injector::shutdown(int signum)
{
    printf("ProMon Injector: Interrupt signal (%d) received.", signum);
    ProMon_logger_close();
    exit(signum);
}

/*
 * First phase of of instrumentation.
 * By this function, injector instruments in one of the three modes (create, attach and open).
 */
BPatch_addressSpace *Injector::loadApplicationImage(accessType_t accessType,
                                                    const char *name, int pid, // For attach
                                                    const char *argv[])
{

    BPatch_addressSpace *handle = NULL;
    switch (accessType) {
        case create:
            handle = bpatch.processCreate(name, argv);
            break;
        case attach:
            handle = bpatch.processAttach(name, pid);
            break;
        case open:
            handle = bpatch.openBinary(name);
            break;
    }
    return handle;
}

/*
 * The second phase of instrumentation.
 * injector, reads the instrumentation XML file, loads the ProMon probe library and perform the instrumentation
 * based on the input data.
 */
void Injector::startInstrumentation(BPatch_addressSpace *app, string strArgv, vector <instRecord> arrangedrecs)
{
    string line;

    bool load = false;
    std::vector < BPatch_function * > functions;
    std::vector < BPatch_function * > probeFunctions;
    std::vector < BPatch_point * > *points;
    BPatch_image *appImage = app->getImage();

    /*
     * Home for ProMon
     * Makefile mandates the user to define it
     */
    char *envVar = getenv("PROMONHOME");
    if (envVar == NULL) {
        ProMon_logger(PROMON_ERROR, "ProMon Injector: PROMONHOME environment variable is not defined!!");
        return;
    }

    string HOME = envVar;
    ///////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////Find library and find probe functions
    /*
     * Load Probe library.
     * relative path is given.
     */
    load = app->loadLibrary((HOME + "/probe/libProMon.so").c_str());

    if (load)
        ProMon_logger(PROMON_DEBUG, "ProMon Injector: Prob library was successfully loaded (%d)");
    else
        ProMon_logger(PROMON_DEBUG, "ProMon Injector: Prob library was NOT loaded (%d)");

    /*
     * Find(1) the function sendInstRecord in the prob library
     * probeFunctions[0] is "sendInstRecord"
     */
    load = appImage->findFunction("sendInstRecord", probeFunctions);

    if (load)
        ProMon_logger(PROMON_DEBUG, "ProMon Injector: The method probe function sendInstRecord was found. (%d)");
    else
        ProMon_logger(PROMON_DEBUG, "ProMon Injector: The method probe function sendInstRecord was NOT found!! (%d)");

    /*
     * Find(2) the function accessDataInst in the prob library
     * probeFunctions[1] is "accessDataInst"
     */
    load = appImage->findFunction("accessDataInst", probeFunctions);

    if (load)
        ProMon_logger(PROMON_DEBUG, "ProMon Injector: The method probe function accessDataInst was found.(%d)");
    else
        ProMon_logger(PROMON_DEBUG, "ProMon Injector: The method probe function accessDataInst was NOT found!!(%d)");


    //////////////////////////////////////////////////////////////////////////////////////
    /////////instrument the defined function in xml file with probe functions/////////////

    /*
     * all records are provided to injector by getRecord() function.
     *
     */
    for (vector<instRecord>::iterator it = arrangedrecs.begin();
         it != arrangedrecs.end(); ++it) {

        /*
         * Find the function in the running application.
         * The details on how instrument the function is defined in the input XML data
         * Last parameter, ensures that regular expression is used.
         */

        load = appImage->findFunction(it->function.c_str(), functions, true, true, true);

        ProMon_logger(PROMON_DEBUG, "ProMon Injector: instrumenting *function==> %s *position==>%s *name==>%s ",
                      it->function.c_str(), it->position.c_str(), it->name.c_str());
        if (load) {
            ProMon_logger(PROMON_DEBUG, "--->found. total: %d ", functions.size());
        } else {
            ProMon_logger(PROMON_ERROR, "--->NOT found!!");
            exit(-1);
        }

#ifdef DEBUG
        for(unsigned int f=0; f<functions.size(); f++)
      {
         cout<<functions[f]->getName()<<" ("<<functions[f]->isInstrumentable()<<")";
         std::set <BPatch_basicBlock*> basicblocks;
         BPatch_flowGraph *fg = functions[f]->getCFG();
         fg->getAllBasicBlocks(basicblocks);
         printf(" BasicBlocks: %d", (int)basicblocks.size());
         std::vector <BPatch_basicBlockLoop*> loops;
         fg->getLoops(loops);
         printf(" Loops %d", (int)loops.size());
         std::vector <BPatch_basicBlockLoop*> loops2;
         fg->getOuterLoops(loops2);
         printf(" OuterLoops %d\n", (int)loops2.size());
      }
#endif

        /*
         * We found the probe functions from the probe library. They have the same FIRST parameter (*tag) in common.
         * We create their arguments. The details about what to be passed is in the injector's XML input file.
         * We find the function that we want to instrument and based on that pass the needed parameters.
         * We'll find out which probe function to use and We instrument the function by the probe function and it's arguments.
         */
        BPatch_Vector < BPatch_snippet * > args; //contains all the parameters
        BPatch_constExpr prgArgs(strArgv.c_str()); //arg parameter for probe function
        args.push_back(&prgArgs);
        BPatch_constExpr tag(it->name.c_str()); //first parameter (common)
        args.push_back(&tag);
        ProMon_logger(PROMON_DEBUG, "prgargs: %s, tag:%s",
                      strArgv.c_str(), it->name.c_str());

        /*
         * Data access instrumentation function has a second parameter.
         * The second parameter is the address of the variable to access
         * Notice that secondParam is created by New keyword. It has to be
         * deleted at end. And it can not be only created in the if block.
         */
        BPatch_snippet *secondParam = NULL;
        if ((string(it->name)).find(DATA_ACCESS) != string::npos) {
            BPatch_variableExpr *variable = appImage->findVariable(it->variableName.c_str());
            secondParam = new BPatch_constExpr(variable->getBaseAddr());
            args.push_back(secondParam);
            ProMon_logger(PROMON_DEBUG, "ProMon Injector: the address in injector: %p ",
                          variable->getBaseAddr());
        }


        /* probe function 0 that is sendInstRecord. It could get extra args but no harm */
        BPatch_funcCallExpr probeFunction0(*probeFunctions[0], args);

        /* probe function 1 that is accessDataInst */
        BPatch_funcCallExpr probeFunction1(*probeFunctions[1], args);

        /*
         * We need to find out which probe function to use.
         */
        BPatch_funcCallExpr *probeFunction;
        if ((string(it->name)).find(DATA_ACCESS) == string::npos)
            probeFunction = &probeFunction0;
        else
            probeFunction = &probeFunction1;

        BPatchSnippetHandle *sh;

        /*
         * If no basic block is defined to be instrumented then
         * instrument the function.
         */
        if (it->basicBlockNo == -1 && it->loopNo.empty()) {
            /*
             * There could be multiple functions with same name
             * So the instrumentation should be done each of them
             */
            for (unsigned int f = 0; f < functions.size(); f++) {
                /*
                * Instrument at begin or end based on position provided
                */
                if (strcmp(it->position.c_str(), "BEGIN") == 0) {
                    points = functions[f]->findPoint(BPatch_entry);
                    sh = app->insertSnippet(*probeFunction, *points, BPatch_callBefore);
                } else {
                    points = functions[f]->findPoint(BPatch_exit);
                    sh = app->insertSnippet(*probeFunction, *points, BPatch_callAfter);
                }
                ProMon_logger(PROMON_DEBUG, "ProMon Injector: insert probe at %s %s position %p",
                              it->position.c_str(), it->name.c_str(), sh);
            }

        } else if (it->basicBlockNo != -1) {
            set < BPatch_basicBlock * > basicblocks;
            for (unsigned int f = 0; f < functions.size(); f++) {

                /*
                 * Get the control flow graph and
                 * get all the basic blocks related to that graph
                 */
                BPatch_flowGraph *fg = functions[f]->getCFG();
                fg->getAllBasicBlocks(basicblocks);

                /*
                 * Traverse to the defined basic block and
                 * inject the code there.
                 * The instrumentation position is entry point of the basic block.
                 */
                set<BPatch_basicBlock *>::iterator block_iter;
                block_iter = basicblocks.begin();
                for (int i = 0; i < it->basicBlockNo; i++)
                    block_iter++;
                BPatch_basicBlock *block = *block_iter;

                /*
                 * Instrument at begin or end based on position provided
                 */
                if (strcmp(it->position.c_str(), "BEGIN") == 0)
                    sh = app->insertSnippet(*probeFunction, *block->findEntryPoint(), BPatch_callBefore);
                else
                    sh = app->insertSnippet(*probeFunction, *block->findExitPoint(), BPatch_callAfter);

                ProMon_logger(PROMON_DEBUG, "ProMon Injector: insert probe at basic block %d %s position: %p %s",
                              it->basicBlockNo, it->position.c_str(), sh, it->name.c_str());
            }


        } else /*handling loops*/
        {
            vector <BPatch_basicBlockLoop *> loops;
            for (unsigned int f = 0; f < functions.size(); f++) {
                /*
                * Get the control flow graph and
                * get all loops related to that graph
                */
                BPatch_flowGraph *fg = functions[f]->getCFG();
                fg->getLoops(loops);

                /*
                 * Find a loop by its name
                 * Dyninst names the loop internally in the format loop_1,loop_1.1 ..etc
                 */
                BPatch_basicBlockLoop *loop = fg->findLoop(("loop_" + string(it->loopNo)).c_str());
                if (loop) {
                    /*
                     *  Get the instrumentation point from control flow graph and  insert snippet
                     *  Instrument at begin or end based on position provided
                     */
                    if (strcmp(it->position.c_str(), "BEGIN") == 0)
                        points = it->eachIteration ? fg->findLoopInstPoints(BPatch_locLoopStartIter, loop)
                            : fg->findLoopInstPoints(BPatch_locLoopEntry, loop);
                    else
                        points = it->eachIteration ? fg->findLoopInstPoints(BPatch_locLoopEndIter, loop)
                            : fg->findLoopInstPoints(BPatch_locLoopExit, loop);

                    sh = app->insertSnippet(*probeFunction, *points);

                    if (it->eachIteration)
                        ProMon_logger(PROMON_DEBUG, "ProMon Injector: insert probe at %s %s position %p",
                                      it->position.c_str(), it->name.c_str(), sh);
                    else
                        ProMon_logger(PROMON_DEBUG,
                                      "ProMon Injector: insert probe at %s (each iteration) %s position %p",
                                      it->position.c_str(), it->name.c_str(), sh);

                }
            }
        }


        /* Clear the list of functions before going to next loop
         * Because each monitoring element is operated separatedly and can be from different function
         */
        functions.clear();

        /* Clear the list of points before going to next loop
         */
        points->clear();

        /*
         * We may have created the second parameter for probe function.
         * We need to delete it before going to next loop.
         */
        if (secondParam != NULL)
            delete secondParam;
    }
}

/*
 * The last phase of instrumentation.
 * Running this function indicates that instrumentation was performed successfully
 */
void Injector::finishInstrumenting(BPatch_addressSpace *app, const char *newName,
                                   const char *dynamicType = NULL)
{
    BPatch_process *appProc = dynamic_cast<BPatch_process *>(app);
    BPatch_binaryEdit *appBin = dynamic_cast<BPatch_binaryEdit *>(app);

    if (appProc) {
        if (strcmp("DynamicInst", dynamicType) == 0) {
            appProc->continueExecution();
            while (!appProc->isTerminated()) {
                bpatch.waitForStatusChange();
            }
        } else if (strcmp("AttachInst", dynamicType) == 0) {
            appProc->detach(true);
        }

        ProMon_logger(PROMON_DEBUG, "I did some dynamic instrumentation.");
    }
    if (appBin) {
        appBin->writeFile(newName);

        ProMon_logger(PROMON_DEBUG, "I did some binary instrumentation.");
    }
}

void Injector::parseApplicationParameter(int argc, char *argv[])
{
    /* Check what type of instrumentation is it */
    shift = 0;
    if (strcmp("BinaryInst", argv[1]) == 0)
        shift = 4;
    else if (strcmp("DynamicInst", argv[1]) == 0)
        shift = 4;
    else if (strcmp("AttachInst", argv[1]) == 0)
        shift = 5;


    /*
     * first arg is the name of the application
     * strArgv: Contains all the parameters of the application including the application name.
     */
    progArgv[0] = argv[shift - 1];
    strArgv = argv[shift - 1];
    /* lets remove the path from the app name */
    unsigned found = strArgv.find_last_of("/\\");
    strArgv = strArgv.substr(found + 1);
    int i = shift;
    for (; i < argc; i++) {
        strArgv += ":";
        progArgv[i - (shift - 1)] = argv[i];
        string temp = argv[i];
        /*
         * Make sure all relative paths are change to absolute.
         */
        if (temp.find("./") != string::npos || temp.find("../") != string::npos) {
            char buf[PATH_MAX]; //defined in limits.h
            char *res = realpath(temp.c_str(), buf);
            if (!res) {
                ProMon_logger(PROMON_ERROR, "Injector: Could not create absolute path!!");
                exit(-1);
            }
            strArgv += buf;
        } else
            strArgv += argv[i];
    }

    /*
     * The last element of an application argument has to be NULL
     * otherwise, the program will not run.
     */
    progArgv[i - (shift - 1)] = NULL;

}

void Injector::instrument(int argc, char *argv[])
{
    /*
     * Read the configuration file and set those environment variable in the file
     */
    readConfigurationFile();

    /*
     * Read the injector input xml file.
     * Files are all in pminput directory.
     */
    Parser parser;
    parser.parse(argv[2]);
    vector <instRecord> arrangedrecs = parser.getRecord();

    /* Parse the parameter and set progArgv and strArgv class property */
    parseApplicationParameter(argc, argv);

    /*
     * The first phase of instrumentation.
     * load the application first.
     * either it is a binary instrumentation or dynamic instrumention.
     */
    BPatch_addressSpace *app;
    if (strcmp("BinaryInst", argv[1]) == 0)
        app = loadApplicationImage(open, argv[shift - 1], 0, progArgv);
    else if (strcmp("DynamicInst", argv[1]) == 0)
        app = loadApplicationImage(create, argv[shift - 1], 0, progArgv);
    else //if(strcmp("AttachInst", argv[1]) == 0)
        app = loadApplicationImage(create, argv[shift - 1], atoi(argv[shift]), progArgv);

    /*
     * The second phase is instrumentation.
     * This part, instrument the application based on the instrumentation instruction input (previously called MonToolLang)
     */
    startInstrumentation(app, strArgv, arrangedrecs);

    /*
     * Finalize the instrumentation.
     */
    string newBin = argv[3] + string(".promon");
    finishInstrumenting(app, newBin.c_str(), argv[1]);

}

int main(int argc, char *argv[])
{
    /*
     * Initializing the log file
     */
    ProMon_logger_init("LOG_injector");

    /*
     * Make sure no file kept open after terminating
     */
    signal(SIGINT, Injector::shutdown_static);


    if ((argc - 1) < 2) {
        ProMon_logger(PROMON_ERROR,
                      "Dynamic instrumentation usage: %s DynamicInst <monspec-xml> <app-binary> [app-argument1 app-argument2 app-argument3 ...]\n \
                      Static instrumentation usage: %s BinaryInst  <monspec-xml> <app-binary> [app-argument1 app-argument2 app-argument3 ...]\n \
                      Attach instrumentation usage: %s AttachInst  <monspec-xml> <app-binary> <pid> [app-argument1 app-argument2 app-argument3 ...]",
                      argv[0], argv[0], argv[0]);
        return 1;
    } else if ((strcmp("DynamicInst", argv[1]) != 0) && (strcmp("BinaryInst", argv[1]) != 0) && (strcmp("AttachInst",
                                                                                                        argv[1]) != 0)) {
        ProMon_logger(PROMON_ERROR,
                      "Dynamic instrumentation usage: %s DynamicInst <monspec-xml> <app-binary> [app-argument1 app-argument2 app-argument3 ...]\n \
                      Static instrumentation usage: %s BinaryInst  <monspec-xml> <app-binary> [app-argument1 app-argument2 app-argument3 ...]\n \
                      Attach instrumentation usage: %s AttachInst  <monspec-xml> <app-binary> <pid> [app-argument1 app-argument2 app-argument3 ...]",
                      argv[0], argv[0], argv[0]);
        return 1;
    }


    Injector injector;
    injector.instrument(argc, argv);

    /*
     * This is a bug. I am still investigating about it.
     * The issue is that sometimes ProMon does not detach from the running application to end the run
     * In that case the run goes to state of wait.
     */
    //if(strcmp("DynamicInst", argv[1]) == 0)
    //{
    //   kill(getpid(), SIGTERM);
    //}

    ProMon_logger_close();
    return 0;
}

