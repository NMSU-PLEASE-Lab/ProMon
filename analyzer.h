#ifndef ANALYZER_H
#define ANALYZER_H

#include <stdio.h>
#include <stdlib.h>   /* needed for os x */
#include <string.h>   /* for memset */
#include <sys/socket.h>
#include <arpa/inet.h>   /* for printing an Internet address in a user-friendly way */
#include <netinet/in.h>
#include <errno.h>   /* defines ERESTART, EINTR */
#include <sys/wait.h>    /* defines WNOHANG, for wait() */
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stack>
#include <map>
#include <vector>
#include <iostream>
#include <list>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include "predefs.h"
#include "util.h"
#include "server.h"
#include "serverMsgQueue.h"
#include "serverEPoll.h"
#include "serverUDP.h"
#include "serverTCP.h"

using namespace std;
extern int errno;

/*
 * LogItem represent a log records from received instrumentation records
 */
struct LogItem
{
   long nanoseconds;
   long seconds;
   string eventName;
   string eventID;
   /* 
    * Contains the lastest event counter from monitored application 
    */
   int appCount; 
   /* 
    * Contains the lastest event counter by analyzer
    * (appCount - analyzerCount = Missing Events
    */
   int analyzerCount; 
};

struct ApplicationDetails
{
   /*
    * A handle to file that all received records will be saved.
    */
   FILE *log_record;

   /*
    * Name of the monitored application
    */
   char appName[100];

   /*
    * Path to monitored application's input files
    */
   char path[100];

   /*
    * Application's parameters.
    */
   string parameters;

   /*
    * Map saves records with its details using LogItem structure
    */
   map<string, LogItem> m;

   /*
    * This map saves the number of arrived single tag records
    */
   map<string, int> singleRecTags;

   /*
    * All counts are occurred in the monitored application.
    * This vector holds them to calculate the time spent with each dual events
    */
   vector<LogItem> countHolder;

   /*
    * The start time (seconds) of the running application
    */
   long beginSeconds; //contains the second what application started
   /*
    * The start time (nanoseconds) of the running application
    */
   long beginNanoseconds; //contains the nanoseconds of the time that application started

   /*
    * constructor
    */
   ApplicationDetails()
   {
      log_record = NULL;
      beginSeconds = 0; //contains the second what application started
      beginNanoseconds = 0; //contains the nanoseconds of the time that application started

   }
};

class Analyzer
{
   public:
   Analyzer(){itself=this;};
   void handleMSG(char *msg, int msgLength);
   void sumup_clean(const char *rank, const char *job_id);
   void start();
   void shutdown(int num);
   static void shutdown_static(int num);
   static void start_static();
   static void handleMSG_static(char *msg, int msgLength);
   static void sumup_clean_static(const char *rank, const char *job_id);

   private:
   void handlingDualEventBegin(const char *rank, const char *job_id);
   void handlingDualEventEnd(const char *rank, const char *job_id);
   void setupLogFiles(const char *rank, const char *job_id);
   void firstRecord();
   void handlingSingleEvent(const char *rank, const char *job_id);
   void handlingProgrammableEvent(const char *rank, const char *job_id);

   /* handle the shutdown */
   static Analyzer *itself; 

   /*
    * This map contains details related to each node of an app.
    * The number of nodes are vary but each node is distinguished by
    * iob_id:node_id
    */
   map<string, ApplicationDetails*> monitoredApps;

   /*
    * pointer to the current application that its record received.
    */
   ApplicationDetails *appDetails;
   
   /* The name of the last event received */
   string eventName;

   /* The time details of the last event */
   long lSeconds, lNanoseconds;

   /* Contains the communication protocl type */
   char* Comm_type;
};


#endif /*ANALYZER_H*/
