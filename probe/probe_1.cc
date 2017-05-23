#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <map>
#include <string>
#include <iostream>
#include <pthread.h>

/*
 * The code can be compiled in parallel (mpi) or serial.
 */
#ifdef MT_WITH_MPI
#include <mpi.h>
#endif

/*
 * Some configurations are defined in predefs.h.
 * for example HEARTBEAT tag is defined in predefs.h
 */
#include "../predefs.h"

using namespace std;

/*
 * This variable defines if the node will be sending information to analyzer
 */
static int heartbeatFactor = 1;

/*
 * This variable defines if the node will be sending information to analyzer
 */
static int nodeFactor = 1;

/*
 * Global variables are defined static so they are global to this file only
 * Number of account for each processor
 */
static long eventCount = 0;
/*
 * The number of heartbeats for each processor
 */
static long heartbeatCount = 0;
/*
 * The rank of the processor (MPI). 0 for serial runs.
 */
static int rank = 0;
/*
 * The job id of the running processor
 */
const static char *job_id = NULL;
/*
 * The user name that is running this job
 */
static char *userName = NULL;
/*
 * The job management system that is running this job
 */
const static char *jobMS = NULL;
/*
 * Copy the network address part of the structure to the
 * sockaddr_in structure which is passed to connect()
 */
static struct sockaddr_in si_other;
/*
 * Specifies the length of the sockaddr_in structure
 */
static int slen = sizeof(si_other);

/*
 * socket description. -1 shows that the socket was not created.
 */
static int s = -1;

/*
 * IP number of the analyzer
 */
static char *IP;

/*
 * Port number to connect to analyzer
 */
static char *PORT;

/*
 * Event ID generator
 */
static map<string, int> eventIDs;

/*
 * Events counter. This counter is increased by DUAL_END
 */
static map<string, int> counters;

/*
 * There are two counters handled by the probes. 
 * both have to be thread safly changed.
 */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * This function creates a tcp/ip socket to transfer data.
 * for any particular process, only one socket is created.
 * The socket will be closed at end of the process life.
 */
static int openSocketTCP(char *ip, char *port)
{
   /*
    * check if the socket is created.
    * if created, then just return.
    * else, create a socket.
    */
   if (s != -1)
      return -1;

   if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      fprintf(stderr, "ProMon ProbeLib: Creating TCP/IP socket() failed!!!\n");
      //exit(1);
   }

   memset((char *) &si_other, 0, sizeof(si_other));
   /*
    * TCP/IP or UDP address family sockets
    * We are using UDP to avoid big traffic and gain better performance
    */
   si_other.sin_family = AF_INET;
   si_other.sin_port = htons(atoi(port));
   /*
    * converts the Internet host address SRV_IP from the IPv4 numbers-and-dots notation into binary
    * form (in network byte order) and stores it in the structure that si_other.sin_addr points to.
    */
   if (inet_aton(ip, &si_other.sin_addr) == 0)
   {
      fprintf(stderr, "ProMon ProbeLib: inet_aton() failed!!!\n");
      //exit(1);
   }

   /*
    * Connect to the server and keep the connection while the process is alive.
    */
   if(( connect(s, (struct sockaddr *)&si_other, sizeof(si_other))) < 0)
   {
      fprintf(stderr, "ProMon ProbeLib: TCP connection failed!!!\n");
      close(s);
   }
   else
   {
      #ifdef DEBUG_PROBFUNC
      printf("ProMon ProbeLib: Connection established...\n");
      #endif
   }

   return 0;
}

/*
 * This function creates a UDP socket to transfer data.
 * for any particular process, only one socket is created.
 * The socket will be closed at end of the process life.
 */
static int openSocketUDP(char *ip, char *port)
{
   /*
    * check if the socket is created.
    * if created, then just return.
    * else, create a socket.
    */
   if (s != -1)
      return -1;

   if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
   {
      fprintf(stderr, "ProMon ProbeLib: Creating socket() failed!!!\n");
      //exit(1);
   }

   memset((char *) &si_other, 0, sizeof(si_other));
   /*
    * TCP/IP or UDP address family sockets
    * We are using UDP to avoid big traffic and gain better performance
    */
   si_other.sin_family = AF_INET;
   si_other.sin_port = htons(atoi(port));
   /*
    * converts the Internet host address SRV_IP from the IPv4 numbers-and-dots notation into binary
    * form (in network byte order) and stores it in the structure that si_other.sin_addr points to.
    */
   if (inet_aton(ip, &si_other.sin_addr) == 0)
   {
      fprintf(stderr, "ProMon ProbeLib: inet_aton() failed!!!\n");
      //exit(1);
   }
   return 0;
}

static void closeSocket()
{
   close(s);
   /* stop send and receive */
   shutdown(s, SHUT_RDWR);
   #ifdef DEBUG_PROBFUNC
   printf("Closing descriptor %d\n", s);
   #endif

}

/*
 * Get the username and job id from job management systems.
 * Currently we are supporting SLURM and PBS but more job management systems will be supported.
 * Get the IP address and PORT number of the analyzer to connect. These information are provided by
 * injector at run time.
 */
static void getEnvData()
{
   
   #ifdef MT_WITH_MPI
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   #else
      rank = 0;
   #endif
   
   userName = getenv("USER");
   jobMS = getenv("JOB_SYSTEM");
   if(jobMS == NULL)
   {
      job_id = PREJOBID;
      jobMS = PREJOBMS;
   }
   else if (strcmp(jobMS, "SLURM") == 0)
   {
      job_id = getenv("SLURM_JOB_ID");
   } else if (strcmp(jobMS, "PBS") == 0)
   {
      job_id = getenv("PBS_JOBID");
   }

   /*
    * These two variables are set by injector when it wants to run an application.
    * It reads the data from configuration file.
    */
   PORT = getenv("PROMONPORT");
   IP = getenv("PROMONIP");

   if (PORT == NULL || IP == NULL )
      fprintf(stderr,
            "ProMon ProbeLib: IP and PORT numbers are not set by injector!!!\n");

   /* If this environment variable is not define then it is set to 1 as default */
   char *nodeFact = getenv("POROMON_NODE_FACTOR");
   if(nodeFact != NULL)
      nodeFactor = atoi(nodeFact);

   /* If this environment variable is not define then it is set to 1 as default */
   char *heartbeatFact = getenv("POROMON_HEARTBEAT_FACTOR");
   if(heartbeatFact != NULL)
      heartbeatFactor = atoi(heartbeatFact);
}

/*
 * Count all event types. The count will help us know which event is missing when 
 * records are analyzed by analyzer
 * returns the last counted event.
 */
string countEvents(const char *tag)
{
   pthread_mutex_lock(&mutex);

   string returnVal;
   string key = string(tag);
   char temp[BUF_SIZE];
   
   if(key.find(DUAL_BEGIN) != string::npos)
   {
      key = key.substr(strlen(DUAL_BEGIN)); 
      eventIDs[key] = eventIDs[key] + 1; 
      sprintf (temp, "#%d#0", eventIDs[key]);
      returnVal = string(temp);

   }else if(key.find(DUAL_END) != string::npos)
   {
      key = key.substr(strlen(DUAL_END));
      int n = eventIDs[key];
      eventIDs[key] = eventIDs[key] - 1;
      counters[key] = counters[key] + 1;
      sprintf (temp, "#%d#%d", n, counters[key]);
      returnVal = string(temp);
   }

   pthread_mutex_unlock(&mutex);
   
   return returnVal;
}

/*
 * Creates instrumentation record, send it to the analyzer using TCP/IP socket
 * tag is the main data generated from the running code. It contains tag info and some other data from the application.
 * tag does not have the time that it generated. We create the time here in this function and send the data.
 */
int sendInstRecordTCP(const char *tag)
{
   /*
    * NodeFactor defines if this rank can send data
    */
   if(rank%nodeFactor != 0)
      return -1;

   /*
    * Check if we have created the socket; if not, create one.
    * We really don't want to create socket with generating each instrumentation record.
    */
   if (s == -1)
   {
      getEnvData();
      openSocketTCP(IP, PORT);
   }

   /*
    * We count all events generated.
    */
   eventCount++;

   /*
    * We count all heartbeats separately.
    */
   if (strcmp(tag, HEARTBEAT) == 0)
   {
      heartbeatCount++;
      /*
       * HeartbeatFactor defines if this heartbeat should be sent
       */
      if(heartbeatCount%heartbeatFactor != 0)
         return -1;
   }

   /*
    * This buffer will contain the instrumentation record.
    * time1 is the time that record generated.
    * clock_gettime is used to have very precise time (in nano seconds)
    */
   char dataBuffer[BUF_SIZE];
   struct timespec time1;
   clock_gettime(CLOCK_REALTIME, &time1);

   /*
    * some tags has more information into it in the form of tag:xx:yy .
    * we need to handle that data too.
    */
   char tmp[BUF_SIZE];
   char extra[BUF_SIZE] = "";
   char *pch;
   sprintf(tmp, "%s", tag);
   tag = strtok(tmp, ":");
   pch = strtok(NULL, ":");

   /*
    * Here we handle extra data from tags.
    */
   while (pch != NULL )
   {
      if(strcmp(extra, "") != 0)
         strcat(extra, ":");
      strcat(extra, pch);
      pch = strtok(NULL, ":");
   }

   /*
    * Keep count of all event types. 
    * By this we can trace lost events. 
    * Each event is sent by its count number and counter ID.
    * format: tag#eventID#counter
    */
   string strTag = string(tag) + countEvents(tag);

   /*
    * Here we form the record to send. Every record is ended with "| "
    * non dual events are not counted (simple tag)
    */
   sprintf(dataBuffer, "%d;%s;%s;%s;%ld;%ld;%s;%ld;%ld;%s| ", rank, userName, jobMS,
            job_id, time1.tv_sec, time1.tv_nsec, strTag.c_str(), eventCount, heartbeatCount,
            extra);

   #ifdef DEBUG_PROBFUNC
   printf("\nDATA:%s\n", dataBuffer);
   #endif

   int n = send(s, dataBuffer, strlen(dataBuffer), 0);

   if (n < 0)
      fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));

   /*
    * We will close the socket when we are done.
    * tag ENDAPP shows the end for run.
    */
   if (strncmp(tag, ENDAPP, sizeof(ENDAPP)) == 0)
      closeSocket();

   return 0;
}


/*
 * Creates instrumentation record, send it to the analyzer using UDP 
 * tag is the main data generated from the running code. It contains tag info and some other data from the application.
 * tag does not have the time that it generated. We create the time here in this function and send the data.
 */
int sendInstRecordUDP(const char *tag)
{
   /*
    * NodeFactor defines if this rank can send data
    */
   if(rank%nodeFactor != 0)
      return -1;

   /*
    * Check if we have created the socket; if not, create one.
    * We really don't want to create socket with generating each instrumentation record.
    */
   if (s == -1)
   {
      getEnvData();
      openSocketUDP(IP, PORT);
   }


   /*
    * We count all events generated.
    */
   eventCount++;

   /*
    * We count all heartbeats separately.
    */
   if (strcmp(tag, HEARTBEAT) == 0)
   {
      heartbeatCount++;
      /*
       * HeartbeatFactor defines if this heartbeat should be sent
       */
      if(heartbeatCount%heartbeatFactor != 0)
         return -1;
   }

   /*
    * This buffer will contain the instrumentation record.
    * time1 is the time that record generated.
    * clock_gettime is used to have very precise time (in nano seconds)
    */
   char dataBuffer[BUF_SIZE];
   struct timespec time1;
   clock_gettime(CLOCK_REALTIME, &time1);

   /*
    * some tags has more information into it in the form of tag:xx:yy .
    * we need to handle that data too.
    */
   char tmp[BUF_SIZE];
   char extra[BUF_SIZE] = "";
   char *pch;
   sprintf(tmp, "%s", tag);
   tag = strtok(tmp, ":");
   pch = strtok(NULL, ":");

   /*
    * Here we handle extra data from tags.
    */
   while (pch != NULL )
   {
      if(strcmp(extra, "") != 0)
         strcat(extra, ":");
      strcat(extra, pch);
      pch = strtok(NULL, ":");
   }



   /*
    * Keep count of all event types. 
    * By this we can trace lost events. 
    * Each event is sent by its count number and counter ID.
    * format: tag#eventID#counter
    */
   string strTag = string(tag) + countEvents(tag);

   /*
    * Here we form the record to send. 
    * non dual events are not counted. (simple tag)
    */
   sprintf(dataBuffer, "%d;%s;%s;%s;%ld;%ld;%s;%ld;%ld;%s", rank, userName, jobMS,
            job_id, time1.tv_sec, time1.tv_nsec, strTag.c_str(), eventCount, heartbeatCount,
            extra);

   #ifdef DEBUG_PROBFUNC
   printf("\nDATA:%s\n", dataBuffer);
   #endif

   int n = sendto(s, dataBuffer, strlen(dataBuffer), 0,
         (struct sockaddr *) &si_other, slen);
   if (n < 0)
      fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));

   /*
    * We will close the socket when we are done.
    * tag ENDAPP shows the end for run.
    */
   if (strncmp(tag, ENDAPP, sizeof(ENDAPP)) == 0)
      closeSocket();

   return 0;
}

extern "C"
int sendInstRecord(const char *tag)
{
//   struct timespec time1;
//   clock_gettime(CLOCK_REALTIME, &time1);

   /*
    * Communication type Evniroment variable sets in the Injector
    */
   

  char* envVar1 = getenv("COMM_TYPE");
  if (envVar1 == NULL)
 {
        printf("\n\nProMon Injector: Communication type is not set !!!\n\n");
      return 0;
   }

   if ((strcmp(envVar1,"TCP") == 0) || (strcmp(envVar1,"EPOLL") == 0))
   {
       sendInstRecordTCP(tag);
//       printf("************************************************************** ProMon Injector: TCP  **************************************************************!!!\n");
   } else {
	    sendInstRecordUDP(tag);
          }

   /*#ifdef MT_WITH_TCP
   sendInstRecordTCP(tag);
   #endif

   #ifdef  MT_WITH_UDP
   sendInstRecordUDP(tag);
   #endif

   #ifdef  MT_WITH_EPOLL
   sendInstRecordTCP(tag);
   #endif*/
//   struct timespec time2;
//   clock_gettime(CLOCK_REALTIME, &time2);
//   printf("hadi++++ %ld \n", time2.tv_nsec-time1.tv_nsec);

   return 0;
}

/*
 * This is data access probe function.
 * This function recieves the typical tag to send and the 
 * address of the varaible to read.
 */
extern "C" 
void accessDataInst(const char *tag, void* varAdd) 
{
   #ifdef DEBUG_PROBFUNC
   printf("ProMon ProbeLib: This is the var address : %p\n", varAdd);
   #endif

   /*
    * Data access type is in the format DATA_ACCESS:varType:varName.
    */
   char tmp[BUF_SIZE];
   char varValue[BUF_SIZE] = "";
   char *varType;

   sprintf(tmp, "%s", tag);
   strtok(tmp, ":");            //tagName
   varType = strtok(NULL, ":"); //varType

   if(strcmp(varType, "short") == 0)
   {
      short *temp = static_cast<short*> (varAdd);
      sprintf(varValue, "%hu", *temp);
   }
   else if(strcmp(varType, "int") == 0)
   {
      int *temp = static_cast<int*> (varAdd);
      sprintf(varValue, "%d", *temp);
   }
   else if(strcmp(varType, "long") == 0)
   {
      long *temp = static_cast<long*> (varAdd);
      sprintf(varValue, "%ld", *temp);
   }
   else if(strcmp(varType, "float") == 0)
   {
      float *temp = static_cast<float*> (varAdd);
      sprintf(varValue, "%g", *temp);
   }
   else if(strcmp(varType, "double") == 0)
   {
      double *temp = static_cast<double*> (varAdd);
      sprintf(varValue, "%g", *temp);
   }
   else if(strcmp(varType, "bool") == 0)
   {
      short *temp = static_cast<short*> (varAdd);
      sprintf(varValue, "%s", (*temp ? "TRUE" : "FALSE") );
   }

   string fullTag(tag);
   fullTag+=string(":")+string(varValue);
   sendInstRecord(fullTag.c_str());  
}
