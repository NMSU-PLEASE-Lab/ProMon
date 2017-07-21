#include "analyzer.h"

using namespace std;
//============= static properties and methods of Analyzer ==============

/* static properties needs to be initialized out of the class */
/* keep a pointer to itself for static calls */
Analyzer *Analyzer::itself=NULL;

/* The static shutdown method */
void Analyzer::shutdown_static(int num)
{
   if(itself != NULL)
      itself->shutdown(num);
}

/* The static start method */
void Analyzer::start_static()
{
   if(itself != NULL)
      itself->start();
}

/* The static handleMSG method */
void Analyzer::handleMSG_static(msgpack::object msg)
{
   if(itself != NULL)
      itself->handleMSG(msg);
}

/* The static sum_clean method */
void Analyzer::sumup_clean_static(const char *rank, const char *job_id)
{
   if(itself != NULL)
      itself->sumup_clean(rank, job_id);
}
//============= regular  methods of Analyzer =========================

/*
 * Make sure log files are closed after terminating.
 * shutdown is static and therefore can be used for handling signals.
 */
void Analyzer::shutdown(int num)
{
   ProMon_logger(PROMON_ERROR,"ProMon Analyzer: shutdown in process.");

   map<string, ApplicationDetails*>::iterator it;
   for (it = monitoredApps.begin(); it != monitoredApps.end(); it++)
   {
      appDetails = it->second;
      if(appDetails->log_record != NULL)
         fclose(appDetails->log_record);

      delete(appDetails);
   }

   /* 
    * Let's close the log file (if there is any)
    */
   ProMon_logger_close();
   exit(num);
}


/*
 * This function handles the received instrumentation record from running HPC applications.
 * For every record received by analyzer, handleMSG is called to analyze it.
 * The received event is the form of msgpack object,
 * msgpack reference: https://github.com/msgpack/msgpack-c
 */
void Analyzer::handleMSG(msgpack::object msg)
{
   int rank, position;
   long eventCount;
   string job_id, userName, jobMS, event_name, event_type,eventCategory,variableType,variableValue;
   const char * positionStr;
   char rankStr[15];
   char buffer[26]; //this is some local buffer and there is no need to keep it in predefs.h

   time_t current = time(NULL);
   ctime_r(&current, buffer);
   buffer[24] = '\0';

   msg.via.array.ptr[0].convert(lSeconds);
   msg.via.array.ptr[1].convert(lNanoseconds);
   msg.via.array.ptr[2].convert(rank);
   msg.via.array.ptr[3].convert(job_id);
   msg.via.array.ptr[4].convert(userName);
   msg.via.array.ptr[5].convert(jobMS);
   msg.via.array.ptr[6].convert(event_type);
   msg.via.array.ptr[7].convert(event_name);
   msg.via.array.ptr[8].convert(position);

   msg.via.array.ptr[9].convert(eventCount);
   msg.via.array.ptr[10].convert(eventCategory);
   if(!event_type.compare("DATA_ACCESS")) {
       msg.via.array.ptr[11].convert(variableType);
       msg.via.array.ptr[12].convert(variableValue);
   }


    eventName = event_name;

   /*Convert postion and rank to strings */
   positionStr = position == 0?"BEGIN":"END";
   sprintf(rankStr, "%d", rank);

   ProMon_logger(PROMON_DEBUG, "ProMon Analyzer: %d %s %s %ld %ld ",
          rank, job_id.c_str(), event_name.c_str(), lSeconds, lNanoseconds);

   /*
    * Do we have any record from this job_id and rank number?
    * if this is the first record then construct an ApplicationDetails
    * if not, then retrieve it from the monitoredApps
    * make sure you delete each app after it is done.
    */
   map<string, ApplicationDetails*>::iterator it;
   string key = string(job_id) + ":" + rankStr;
   it = monitoredApps.find(key);
   if (it == monitoredApps.end())
   {
      appDetails = new ApplicationDetails;
      monitoredApps.insert(pair<string, ApplicationDetails*>(key, appDetails));
   } else
   {
      appDetails = monitoredApps[key];
   }

   /*
    * setupLogFiles will always check if log files related to the record received from rank
    * are created. If log files are not created, then create them.
    */
   setupLogFiles(rankStr, job_id.c_str());

   /*
    * Save all received record in the file log_record
    * This statement runs after setupLogFiles() to make sure log_record is not NULL
    */
    if(!event_type.compare("DATA_ACCESS"))
    {
        fprintf(appDetails->log_record, "%ld;%ld;%s;%d;%s;%s;%s;%s;%s;%s;%ld;%s;%s;%s\n",
                lSeconds, lNanoseconds, buffer, rank, job_id.c_str(),
                userName.c_str(), jobMS.c_str(), event_type.c_str(), event_name.c_str(), positionStr, eventCount,
                eventCategory.c_str(),variableType.c_str(),variableValue.c_str());
    }
    else
    {
        fprintf(appDetails->log_record, "%ld;%ld;%s;%d;%s;%s;%s;%s;%s;%s;%ld;%s\n",
                lSeconds, lNanoseconds, buffer, rank, job_id.c_str(),
                userName.c_str(), jobMS.c_str(), event_type.c_str(), event_name.c_str(), positionStr, eventCount,
                eventCategory.c_str());
    }

   fflush(appDetails->log_record);

   /*
    * This is the first record will be received from application. The first record is programmable and 
    * therefore THE optional python script (scripts/appName.py) is called when this event is received.  
    * The first event is sent by ProMon and it is transparent from the user. 
    */
   if (eventName.find(FIRST_RECORD) != string::npos)
   {
      firstRecord();
   }

   /*
    * If the received instrumentation records is of type dual tag, then
    * handlingDualEventBegin will handle the record.
    * This function is called for BEGIN dual tag
    */
   if (eventName.find(DUAL_BEGIN) != string::npos)
   {
      handlingDualEventBegin(rankStr, job_id.c_str());
   }

   /*
    * If the received instrumentation records is of type dual tag, then
    * handlingDualEventEnd will handle the record.
    * This function is called for END dual tag
    */
   if (eventName.find(DUAL_END) != string::npos)
   {
      handlingDualEventEnd(rankStr, job_id.c_str());
   }

   /*
    * Single tag records does not start with Begin_ or End_
    */
   if (eventName.find(DUAL_BEGIN) == string::npos && eventName.find(DUAL_END) == string::npos)
   {
      handlingSingleEvent(rankStr, job_id.c_str());
   }

   /*
    * Events that contains "PROGRAMMABLE" add-on name (defined in predef.h) will run this function.
    * The function calls the optional appName.py python program.
    */
   if (eventName.find(PROGRAMMABLE) != string::npos)
   {
      handlingProgrammableEvent(rankStr, job_id.c_str());
   }

   /*
    * LAST_RECORD indicates that that application is ended.
    * All data needs to be summarized and saved for report.
    */
   if (eventName.find(LAST_RECORD) != string::npos)
   {
      sumup_clean(rankStr, job_id.c_str());
   }

}

/*
 * setupLogFiles will always check if log files related to the record received from rank
 * are created. If log files are not created, then create them
 */
void Analyzer::setupLogFiles(const char *rank, const char *job_id)
{
   /*
    * All node records of a job_id is saved in one log file
    */
   if (appDetails->log_record == NULL)
   {
      /*
       * Home for ProMon
       * Makefile mandates the user to define it
       */
      char* envVar = getenv("PROMONHOME");
      if (envVar == NULL)
      {
         ProMon_logger(PROMON_ERROR, "ProMon Analyzer: PROMONHOME environment variable is not defined!!");
         return;
      }
   
      string HOME = envVar;
      string directory = HOME+ "/" + LOG_DIRECTORY;
      directory = directory.append(job_id);
      mkdir(directory.c_str(), 0777);
      directory = directory.append("/");
      directory = directory.append(LOG_EVENTS);
      ProMon_logger(PROMON_DEBUG, "ProMon Analyzer %4d|%4d: Using %s as log file",
            atoi(rank), atoi(job_id), directory.c_str());
      appDetails->log_record = fopen(directory.c_str(), "a");

      if (appDetails->log_record == NULL)
         ProMon_logger(PROMON_ERROR, "ProMon Analyzer %4d|%4d: failed to open file %s",
               atoi(rank), atoi(job_id), directory.c_str());

   }
}

/*
 * This is the first record will be received from application. The first record is programmable and 
 * therefore THE optional python script (scripts/appName.py) is called when this event is received.  
 * The first event is sent by ProMon and it is transparent from the user. 
 */
void Analyzer::firstRecord()
{
   char* temp = strtok(NULL, ";");
   string extra = temp;
   temp = strtok(temp, ":"); //note:different token.
   sprintf(appDetails->appName, "%s", temp);

   /*
    * The extra data is in this format:
    * appName:Parameters
    * in which Parameters has more data tokenized by ":"
    * We need to remove the part "appName:" from the extra data and
    * send the rest of the data to python script related to running application
    */
   size_t found = extra.find(":");
   if (found != string::npos)
      appDetails->parameters = extra.substr(++found);
}

/*
 * This function will handle dual tags.
 * This function is called at the begin of an event.
 */
void Analyzer::handlingDualEventBegin(const char *rank, const char *job_id)
{
   LogItem li;
   /*
    * The tag is in this format: DUAL_BEGIN+tag#ID_NO#counter (no + sign)
    * Note: tag#ID_NO is the eventID
    */
   string eventID = eventName.substr(strlen(DUAL_BEGIN));
   eventID = eventID.substr(0, eventID.rfind("#"));
   string strEvent = eventID.substr(0, eventID.find("#"));

   if (appDetails->m.count(strEvent) == 0)
   {
      /*
       * Keep track of all received records.
       * analyzerCount and appCount will provide us the missing events of that particular events.
       */
      li.seconds = 0;
      li.nanoseconds = 0;
      li.eventName = strEvent;
      li.analyzerCount = 0; //no record yet.
      li.appCount = atoi(eventID.substr(eventID.rfind("#")+1).c_str()); 
      appDetails->m[strEvent] = li;
   }

   li.seconds = lSeconds;
   li.nanoseconds = lNanoseconds;
   li.eventName = strEvent;
   li.eventID = eventID;
   appDetails->countHolder.push_back(li);

   if (eventName.find(BEGINAPP) != string::npos)
   {
      ProMon_logger(PROMON_DEBUG, "ProMon Analyzer %4d|%4d: Being Application", 
            atoi(rank), atoi(job_id));

      appDetails->beginSeconds = lSeconds;
      appDetails->beginNanoseconds = lNanoseconds;
   }
}

/*
 * This function will handle dual tags.
 * This function is called at the end of an event
 */
void Analyzer::handlingDualEventEnd(const char *rank, const char *job_id)
{
   LogItem li;
   /*
    * The tag is in this format: DUAL_BEGIN+tag#ID_NO#counter (no + sign)
    * Note: tag#ID_NO is the eventID
    */
   string eventID = eventName.substr(strlen(DUAL_END));
   eventID = eventID.substr(0, eventID.rfind("#"));
   int appCount = atoi(eventName.substr(eventName.rfind("#")+1).c_str()); 

   /*
    * With each record regarding end of an event, we check the vector and find when it started
    * each record has a unique event ID (eventname#number)
    */
   size_t index = -1;
   int found = 0;
   for(vector<LogItem>::iterator it = appDetails->countHolder.begin(); 
             it != appDetails->countHolder.end(); it++)
   {
      index++;
      if(it->eventID == eventID)
      {
         found = 1;
         li = *it;
         appDetails->countHolder.erase(it);
         break;
      }
   }

   /*
    * Totall we have two cases for missing dual events: first portion was not captured (Begin_) or end portion is not captured (End_)
    * This is the case that end of the event was not captured.
    */
   if(!found)
   {
      /*
       * The begin of the dual event was not received.
       * we need to find an estimate (based on average) for this event
       */
      return;
   }

   /* Keep count of all calculated received event record */
   appDetails->m[li.eventName].analyzerCount++ ; 
   appDetails->m[li.eventName].appCount = appCount;

   if ((lNanoseconds - li.nanoseconds) < 0)
   {
      appDetails->m[li.eventName].seconds += lSeconds - li.seconds - 1;
      appDetails->m[li.eventName].nanoseconds += PRECISION + lNanoseconds
            - li.nanoseconds;

   } else
   {
      appDetails->m[li.eventName].seconds += lSeconds - li.seconds;
      appDetails->m[li.eventName].nanoseconds += lNanoseconds - li.nanoseconds;

   }

   if (appDetails->m[li.eventName].nanoseconds > PRECISION)
   {
      appDetails->m[li.eventName].seconds += abs(
            appDetails->m[li.eventName].nanoseconds / PRECISION);
      appDetails->m[li.eventName].nanoseconds =
            appDetails->m[li.eventName].nanoseconds % PRECISION;
   }

}

/*
 * All single tag records are counted and saved.
 * Single tag records, help to check the progress of the application while we
 * are not interested to know how much time was spent in those progress.
 * Mainly used for discrete event based applications
 */
void Analyzer::handlingSingleEvent(const char *rank, const char *job_id)
{
   if(appDetails->singleRecTags.find(eventName) == appDetails->singleRecTags.end())
      appDetails->singleRecTags[eventName] = 1; //count begins
   else
      appDetails->singleRecTags[eventName]++;

   /*
    * if the received instrumentation record is Data_Access then
    */
   if (eventName.find(DATA_ACCESS) != string::npos)
   {
      char *varType = strtok(NULL, ":");
      char *varName = strtok(NULL, ":");
      char *varValue = strtok(NULL, ":");
      /*
       * Home for ProMon
       * Makefile mandates the user to define it
       */
      char* envVar = getenv("PROMONHOME");
      if (envVar == NULL)
      {
         ProMon_logger(PROMON_ERROR, "ProMon Analyzer: PROMONHOME environment variable is not defined!!");
         return;
      }

      string HOME = envVar; 
      string directory = HOME + "/" + LOG_DIRECTORY;
      directory = directory.append(job_id);
      directory = directory.append("/");
      directory = directory.append(rank);
      directory = directory.append("/");
      directory = directory.append(LOG_DATA_ACCESS);

      FILE *log_data_access = fopen(directory.c_str(), "a");
      if (log_data_access == NULL)
         ProMon_logger(PROMON_ERROR, "ProMon Analyzer %4d|%4d: failed to open file %s",
               atoi(rank), atoi(job_id), directory.c_str());


      /* Show the time that data received */
      struct tm *current;
      time_t now;
      time(&now);
      current = localtime(&now);
   
      fprintf(log_data_access, "%i:%i:%i   %s(%s) --> %s\n", 
            current->tm_hour, current->tm_min, current->tm_sec, varName, varType, varValue);
      fflush(log_data_access);
      fclose(log_data_access);
   }
}

/*
 * Events that contains "PROGRAMMABLE" add-on name (defined in predef.h) will run this function.
 * The function calls the optional appName.py python program. 
 */
void Analyzer::handlingProgrammableEvent(const char *rank, const char *job_id)
{
   /*
    * Home for ProMon
    * Makefile mandates the user to define it
    */
   char* envVar = getenv("PROMONHOME");
   if (envVar == NULL)
   {
      ProMon_logger(PROMON_DEBUG, "ProMon Analyzer: PROMONHOME environment variable is not defined!!");
      return;
   }

   string HOME = envVar;
   FILE *fp;
   char buf[1024];
   sprintf(buf, "python %s/%s%s.py %s", HOME.c_str(), INPUT_DIRECTORY, appDetails->appName,
         appDetails->parameters.c_str());
   fp = popen(buf, "r");

   /*
    * "fp == NULL" means that python file for the running test does not exist.
    * python script is optional. 
    */
   if (fp == NULL)
   {
      ProMon_logger(PROMON_ERROR, 
            "ProMon Analyzer %4d|%4d: No optional Python script to run.",
            atoi(rank), atoi(job_id));
      pclose(fp);
      return;
   }
}

/*
 * LAST_RECORD indicates that that application is ended.
 * All data needs to be summarized and saved for report.
 */
void Analyzer::sumup_clean(const char *rank, const char *job_id)
{
   /* key is needed to delete the appDetails */
   string key = string(job_id) + ":" + string(rank);
   if ( monitoredApps.find(key) == monitoredApps.end() )
   {
      ProMon_logger(PROMON_DEBUG, "ProMon Analyzer: Data for %s:%s is already summrized and cleaned up!", job_id, rank);
      return;
   }
  
   /* Select the correct appDetails. sumup_clean is also called by communication protocols */
   appDetails = monitoredApps[key];

   /* Home for ProMon. Makefile mandates the user to define it  */
   char* envVar = getenv("PROMONHOME");
   if (envVar == NULL)
   {
      ProMon_logger(PROMON_ERROR, "ProMon Analyzer: PROMONHOME environment variable is not defined!!");
      return;
   }

   string HOME = envVar;
   FILE *summary;
   string directory = HOME + "/" + LOG_DIRECTORY;
   directory = directory.append(job_id);
   directory = directory.append("/");
   directory = directory.append(rank);
   directory = directory.append("/");
   directory = directory.append(LOG_SUMMARY);

   summary = fopen(directory.c_str(), "w");
   if (summary == NULL)
      ProMon_logger(PROMON_ERROR, "ProMon Analyzer %4d|%4d: failed to open file %s",
            atoi(rank), atoi(job_id), directory.c_str());

   fprintf(summary, "Job ID:%s\n", job_id);
   fprintf(summary, "Rank:%s\n", rank);
   fflush(summary);

   map<string, LogItem>::iterator it;
   for (it = appDetails->m.begin(); it != appDetails->m.end(); it++)
   {
      /*
       * Find the number of missing events and estimate  
       * the total time spent on those times
       */
      long secondsAvg = 0, nanosecondsAvg = 0;
      if( (*it).second.appCount != (*it).second.analyzerCount)
      {
         int n = (*it).second.appCount - (*it).second.analyzerCount;
         secondsAvg = (*it).second.seconds/(long)n;
         nanosecondsAvg = (*it).second.nanoseconds/(long)n;
         (*it).second.seconds += secondsAvg;
         (*it).second.nanoseconds += nanosecondsAvg;
         if ((*it).second.nanoseconds > PRECISION)
         {
            (*it).second.seconds += abs((*it).second.nanoseconds / PRECISION);
            (*it).second.nanoseconds = (*it).second.nanoseconds % PRECISION;
         }
         
         fprintf(summary, 
               "! %d missing %s. Avg Seconds: %ld Avg nanoseconds: %ld\n", n, 
               (*it).first.c_str(), secondsAvg, nanosecondsAvg);
      }

      fprintf(summary, "%s:%ld:%ld\n",
            (*it).first.c_str(), (*it).second.seconds,
            (*it).second.nanoseconds);
      fflush(summary);
   }

   /*
    * Report all single tag records into summary file
    */
   map<string, int>::iterator it2;
   for (it2 = appDetails->singleRecTags.begin(); it2 != appDetails->singleRecTags.end(); it2++)
   {
      fprintf(summary, "%s:%d\n",
            (*it2).first.c_str(), (*it2).second);
      fflush(summary);
   }

   fclose(summary);
   fclose(appDetails->log_record);
   appDetails->log_record = NULL;

   ProMon_logger(PROMON_DEBUG, "ProMon Analyzer %4d|%4d: End of Application", 
         atoi(rank), atoi(job_id));

   /*
    * We have to make sure the appDetails is deleted.
    * Every node or application would have its appDetails
    * and erase the pointer from the map
    */
   delete appDetails;
   monitoredApps.erase(monitoredApps.find(key));
}

/*
 * This function wraps the serveTCP or serveUDP. 
 * At compile time it will be defined which function to be wrapped.
 */
void Analyzer::start()
{
   /*
    * PORT and COMM_TYPE should be defined by user
    */
   char *PORT = getenv("PROMONPORT");
   Comm_type = getenv("COMM_TYPE");
   if (PORT == NULL || Comm_type == NULL)
   {
      ProMon_logger(PROMON_ERROR,"ProMon Analyzer: PORT or COMM_TYPE numbers is not defined!!!");
      return;
   }

   int port = atoi(PORT); 

   if ((strcmp(Comm_type,"TCP") == 0))
   {
     serverTCP comm(port);
     comm.start();
   } else if ((strcmp(Comm_type,"UDP") == 0)) 
   {
		serverUDP comm(port);
      comm.start();
	} else if ((strcmp(Comm_type,"EPOLL") == 0))
   {
      serverEPoll comm(port);
      comm.start();
   } else if ((strcmp(Comm_type,"MSGQUEUE") == 0))
   {
      serverMsgQueue comm(port);
      comm.start();
   }
}

int main(int argc, char **argv)
{
   /*
    * Initializing the log file
    */
   ProMon_logger_init("LOG_analyzer");

   /*
    * Read the configuration file and set the variable as environment variable for Analyzer
    */
   readConfigurationFile();
   
   Analyzer analyzer;
   /*
    * Ignore SIGHLD signal to prevent zomie problem
    */
   signal(SIGCHLD,SIG_IGN);
   
   /*
    * Make sure no file kept open after terminating
    */
   signal(SIGINT, Analyzer::shutdown_static);

   /* start the server */ 
   analyzer.start();
}

