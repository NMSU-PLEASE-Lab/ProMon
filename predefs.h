
#ifndef PREDEFINES_H
#define PREDEFINES_H
/*
Configuration file
*/

#define BUF_SIZE 256 //the size of buffer used anywhere in this application

#define PROMON_UDP_PERIODIC_SWEEP 1
#define PROMON_UDP_INACTIVITY_PERIOD 5

//log file names
#define LOG_EVENTS "events.log"
#define LOG_CHEPOINT "checkPoint-"
#define LOG_LAST_ACCOMPLISHED "lastAccomplished.log"
#define LOG_SUMMARY "summary.log"
#define LOG_DIRECTORY  "pmlogs/"
#define INPUT_DIRECTORY "pminput/"
#define SCRIPT_DIRECTORY "scripts/"
#define SETATTRIBUTPYTHON "SetAttributes.py"
#define LOG_DATA_ACCESS "data_access.log"

#define PROMON_ERROR 0
#define PROMON_FATAL 1
#define PROMON_INFO  2
#define PROMON_DEBUG 3

#define PRECISION 1000000000

// if it is a simple mpi application then give it a predefined job id.
#define PREJOBID "123"
#define PREJOBMS "NoJobMS"

//These are related to tags
//this is used in lib, injector and analyzer
#define INPUT_ADDRESS "inputs" //this is the directory of all input files
//#define DUAL_BEGIN "Begin_"
//#define DUAL_END "End_"
#define DUAL_BEGIN "DUAL;Begin;"
#define DUAL_END "DUAL;End;"
#define SINGULAR "SINGULAR;;"
#define APPLICATION "Application"//tag to define begin and end of application
#define HEARTBEAT "HEARTBEAT;;"
#define SETUP "Setup"
#define CHECKPOINT "Checkpoint"
#define DEFENSIVEIO "DefensiveIO"
#define PRODUCTIVEIO "ProductiveIO"
#define RUN "Run"
#define DATA_ACCESS "DATA_ACCESS"
#define PROGRAMMABLE "-PROGRAMMABLE;Program"
#define FIRST_RECORD "DUAL;Begin;RECORD" PROGRAMMABLE
#define LAST_RECORD "DUAL;End;RECORD" PROGRAMMABLE


#define BEGINAPP DUAL_BEGIN APPLICATION
#define ENDAPP  DUAL_END APPLICATION
#define ENDSETUP DUAL_END SETUP
#define ENDCHECKPOINT DUAL_END CHECKPOINT

//This is the size of the tcp queue if ProMon functions in tcp mode.
#define TCP_QUEUE_SIZE 100

#endif /*PREDEFINES_H*/
