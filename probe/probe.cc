#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <mqueue.h>
#include "probe.h"

#define MSGSZ     800


/*
 * The code can be compiled in parallel (mpi) or serial.
 */
#ifdef MT_WITH_MPI
#include <mpi.h>
#endif

/*
 * Some configurations are defined in predefs.h.
 */
#include "../predefs.h"

using namespace std;


/*
 * This variable defines if the node will be sending information to analyzer
 */
static int procLimiter = 1;

/*
 * Global variables are defined static so they are global to this file only
 * Number of account for each processor
 */
static long eventCount = 0;

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
 * The communication protocol. ProMon supports several protocols. 
 */
const static char *comm_type = NULL;

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
 * The running application's path.
 */
static string prgArgs = " ";

/*
 * Called when the application exit execution. Using atexit function.
 * Send the last event and close the connection.
 * maximum 32 function can register with atexit()
 */
void atExit(void)
{
    string temp = "1:1:PROMON:" + string(LAST_RECORD);
//	sendInstRecord(prgArgs.c_str(), temp.c_str());
    closeSocket();
}

/*
 * Similar to above but for Ctrl+C exit. Handling signals.
 */
void atExit(int sigNum)
{
    string temp = "1:1:PROMON:" + string(LAST_RECORD);
//	sendInstRecord(prgArgs.c_str(), temp.c_str());
    closeSocket();
}

/*
 * Called when the application does quick_exit (C++11)
 * It can be configured similar to atexit functionality. 
 * --> Future decision and implementation!
 */
void atQuickExit(void)
{
    string temp = "1:1:PROMON:" + string(LAST_RECORD);
//	sendInstRecord(prgArgs.c_str(), temp.c_str());
    closeSocket();
}

/*
 * Declare the message structure.
 */
typedef struct msgbuf1 {
    long mtype;
    char mtext[MSGSZ];
} message_buf1;

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

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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
    if (inet_aton(ip, &si_other.sin_addr) == 0) {
        fprintf(stderr, "ProMon ProbeLib: inet_aton() failed!!!\n");
        //exit(1);
    }

    /*
     * Connect to the server and keep the connection while the process is alive.
     */
    if ((connect(s, (struct sockaddr *) &si_other, sizeof(si_other))) < 0) {
        fprintf(stderr, "ProMon ProbeLib: TCP connection failed!!!\n");
        close(s);
    } else {
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

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
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
    if (inet_aton(ip, &si_other.sin_addr) == 0) {
        fprintf(stderr, "ProMon ProbeLib: inet_aton() failed!!!\n");
        //exit(1);
    }
    return 0;
}

static void closeSocket()
{
#ifdef DEBUG_PROBFUNC
    printf("Closing descriptor %d\n", s);
#endif

    close(s);
    /* stop send and receive */
    shutdown(s, SHUT_RDWR);
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
    if (jobMS == NULL) {
        job_id = PREJOBID;
        jobMS = PREJOBMS;
    } else if (strcmp(jobMS, "SLURM") == 0) {
        if (getenv("SLURM_JOB_ID") != NULL)
            job_id = getenv("SLURM_JOB_ID");
        else
            job_id = "NULL";
    } else if (strcmp(jobMS, "PBS") == 0) {
        if (getenv("PBS_JOBID") != NULL)
            job_id = getenv("PBS_JOBID");
        else
            job_id = "NULL";
    }

    /*
     * These two variables are set by injector when it wants to run an application.
     * It reads the data from configuration file.
     */
    PORT = getenv("PROMONPORT");
    IP = getenv("PROMONIP");

    if (PORT == NULL || IP == NULL)
        fprintf(stderr,
                "ProMon ProbeLib: IP and PORT numbers are not set by injector!!!\n");

    /* If this environment variable is not define then it is set to 1 as default */
    char *nodeFact = getenv("PROMON_PROC_LIMITER");
    if (nodeFact != NULL)
        procLimiter = atoi(nodeFact);

}

/*
 * Work in progress. I still don't know how to program for rate. The dual events are hard to systimatically limit.
 * PML's SamplingRate, Priority, and Category implementation.
 * Return: -1 to eleminate the event and 0 to send it.
 */
static int getEnvData(const char *eventRate, const char *eventPriority, const char *eventCategory)
{
    comm_type = getenv("COMM_TYPE");
    char *rate = getenv("PROMON_SAMPLING_RATE");
    char *priority = getenv("PROMON_BLOCK_PRIORITY");
    char *category = getenv("PROMON_BLOCK_CATEGORY");
    if (comm_type == NULL) {
        fprintf(stderr, "ProMon ProbLib: Communication protocol is not set !!!\n\n");
        exit(1);
    }

    if (rate == NULL && priority == NULL && category == NULL)
        return 0;

    /* Priorities are from 0 to 9 only */
    if (priority != NULL) {
        if (strstr(priority, eventPriority) != NULL) //strstr parameters is not how are used to (it is in reverse)!!!!
            return -1;
    }

    if (category != NULL) {
        if (strstr(category, eventCategory) != NULL)
            return -1;
    }
    /* No issue then let the event dispatched.
     * return 0 is the ok from getEnvData function
     */
    return 0;
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

    if (key.find(DUAL_BEGIN) != string::npos) {
        key = key.substr(strlen(DUAL_BEGIN));
        eventIDs[key] = eventIDs[key] + 1;
        sprintf(temp, "#%d#0", eventIDs[key]);
        returnVal = string(temp);

    } else if (key.find(DUAL_END) != string::npos) {
        key = key.substr(strlen(DUAL_END));
        int n = eventIDs[key];
        eventIDs[key] = eventIDs[key] - 1;
        counters[key] = counters[key] + 1;
        sprintf(temp, "#%d#%d", n, counters[key]);
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
int sendInstRecordTCP(eventRecord *record)
{

    /*
     * Check if we have created the socket; if not, create one.
     * We really don't want to create socket with generating each instrumentation record.
     */
    if (s == -1) {
        getEnvData();
        openSocketTCP(IP, PORT);
    }


    /*
     * We count all events generated.
     */
    eventCount++;

    /*
     * time1 is the time that record generated.
     * clock_gettime is used to have very precise time (in nano seconds)
     */
    struct timespec time1;
    clock_gettime(CLOCK_REALTIME, &time1);
    record->timeSec = time1.tv_sec;
    record->timeNanoSec = time1.tv_nsec;
    record->rank = rank;
    record->username = userName;
    record->jobMS = jobMS;
    record->jobId = job_id;
    record->eventCount = eventCount;

#ifdef DEBUG_PROBFUNC
    cout<<"TimeSec:"<<record->timeSec<<" ";
    cout<<"TimeNanoSec:"<<record->timeNanoSec<<" ";
    cout<<"Rank:"<<record->rank<<" ";
    cout<<"Username:"<<record->username<<" ";
    cout<<"JobMS:"<<record->jobMS<<" ";
    cout<<"JobId:"<<record->jobId<<" ";
    cout<<"EventName:"<<record->eventName<<" ";
    cout<<"EventType:"<<record->eventType<<" ";
    cout<<"EventCategory:"<<record->eventCategory<<" ";
    cout<<"EventPosition:"<<record->eventPosition<<" ";
    cout<<"EventCount:"<<record->eventCount<<" ";
#endif

//    int n = send(s, dataBuffer, strlen(dataBuffer), 0);
    int n = send(s, &record, sizeof(struct eventRecord), 0);


    if (n < 0)
        fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));

    return 0;
}

/*
 * Creates instrumentation record, send it to the LDMS using IPC - Message Queue socket
 * tag is the main data generated from the running code. It contains tag info and some other data from the application.
 * tag does not have the time that it generated. We create the time here in this function and send the data.
 */
int sendInstRecordMSGQUEUE(const char *tag)
{
    //   /*
    //    * Check if we have created the socket; if not, create one.
    //    * We really don't want to create socket with generating each instrumentation record.
    //    */
    //   if (s == -1)
    //   {
    getEnvData();
    //      openSocketTCP(IP, PORT);
    //   }

    /*
     * We count all events generated.
     */
    eventCount++;


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
    while (pch != NULL) {
        if (strcmp(extra, "") != 0)
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
    string strTag = string(tag); //+ countEvents(tag);

    /*
     * Here we form the record to send. Every record is ended with "| "
     * non dual events are not counted (simple tag)
//	 * "",0 before last is the variable type and variable value
     */
    sprintf(dataBuffer, "%d;%s;%s;%s;%ld;%ld;%ld;%s;%ld;%d;%s;%d;%s| ", rank, userName, jobMS,
            job_id, time1.tv_sec, ((time1.tv_sec * 1000) + (time1.tv_nsec / 1000000)),
            time1.tv_nsec, strTag.c_str(), eventCount, 0, "", 0, extra);

#ifdef DEBUG_PROBFUNC
    printf("\nDATA:%s\n", dataBuffer);
#endif

    /*
     Send a message using Message Queue to the LDMS
     */
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf1 sbuf;
    size_t buf_length;

    key = 1978;

    if ((msqid = msgget(key, msgflg)) < 0) {
        fprintf(stderr, "ProMon ProbeLib: Error message queue can not be established");
        exit(1);
    }

    sbuf.mtype = 1; //send message type 1

    // Copy the dataBuffer to the message structure
    strcpy(sbuf.mtext, dataBuffer);
    buf_length = strlen(sbuf.mtext) + 1;

    /*
     * Send the message
     */
    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
        fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));
        //exit(1);
    }

    return 0;
}

/*
 * Creates instrumentation record, send it to the LDMS using POSIX 
 * tag is the main data generated from the running code. It contains tag info and some other data from the application.
 * tag does not have the time that it generated. We create the time here in this function and send the data.
 */
int sendInstRecordPOSIX(const char *tag)
{
    //   /*
    //    * Check if we have created the socket; if not, create one.
    //    * We really don't want to create socket with generating each instrumentation record.
    //    */
    //   if (s == -1)
    //   {
    getEnvData();
    //      openSocketTCP(IP, PORT);
    //   }

    /*
     * We count all events generated.
     */
    eventCount++;


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
    while (pch != NULL) {
        if (strcmp(extra, "") != 0)
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
    string strTag = string(tag); // + countEvents(tag);

    /*
     * Here we form the record to send. Every record is ended with "| "
     * non dual events are not counted (simple tag)
     */
    sprintf(dataBuffer, "%d;%s;%s;%s;%ld;%ld;%s;%ld;%d;%s| ", rank, userName, jobMS,
            job_id, time1.tv_sec, time1.tv_nsec, strTag.c_str(), eventCount, 0,
            extra);

#ifdef DEBUG_PROBFUNC
    printf("\nDATA:%s\n", dataBuffer);
#endif

    mqd_t mqdes; // Message queue descriptors

    //    struct mq_attr attr;
    //    /* initialize the queue attributes */
    //    attr.mq_flags = 0;
    //    attr.mq_maxmsg = 10;
    //    attr.mq_msgsize = MSGSZ;

    fprintf(stderr, "**** Try to open the queue ****\n");
    /* open the mail queue */
    mqdes = mq_open("/Promon__queue", O_WRONLY);

    if (mqdes == -1) {
        fprintf(stderr, "ProMon ProbeLib: Error opening the POSIX QUEUE %s:%d: \n", __func__, __LINE__);
        //exit(1);
    }

    fprintf(stderr, "**** Try to SEND the data ****\n");
    // Send the event
    if (mq_send(mqdes, (char *) &dataBuffer, sizeof(dataBuffer), 0) == -1) {
        fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));
    }


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
     * Check if we have created the socket; if not, create one.
     * We really don't want to create socket with generating each instrumentation record.
     */
    if (s == -1) {
        getEnvData();
        openSocketUDP(IP, PORT);
    }


    /*
     * We count all events generated.
     */
    eventCount++;

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
    while (pch != NULL) {
        if (strcmp(extra, "") != 0)
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
    sprintf(dataBuffer, "%d;%s;%s;%s;%ld;%ld;%s;%ld;%d;%s", rank, userName, jobMS,
            job_id, time1.tv_sec, time1.tv_nsec, strTag.c_str(), eventCount, 0,
            extra);

#ifdef DEBUG_PROBFUNC
    printf("\nDATA:%s\n", dataBuffer);
#endif

    int n = sendto(s, dataBuffer, strlen(dataBuffer), 0,
                   (struct sockaddr *) &si_other, slen);
    if (n < 0)
        fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));

    return 0;
}

extern "C"
int sendInstRecord(const char *args, const char *name, const char *type, const char *category, unsigned int priority,
                   const char *position, unsigned int samplingRate)
{

    /*
     * procLimiter defines if this rank can send data
     */
    if (rank % procLimiter != 0)
        return -1;

    /* Keep a all args here. ProMon will send it one time to inform Analyzer what are the args */
    if (prgArgs == " ") {
        /* atExit will be called when the process exits. */
        atexit(atExit);
        /* Make sure no file kept open after terminating */
        signal(SIGINT, atExit);
        prgArgs = args;
        string temp = "1:1:PROMON:" + string(FIRST_RECORD) + ":" + prgArgs;
        //sendInstRecord(prgArgs.c_str(), temp.c_str());
    }
    struct eventRecord record;
    record.eventName.assign(name);
    record.eventType.assign(type);
    record.eventCategory = category;
    record.eventPosition.assign(position);
    char priorityString[5];
    char samplingRateString[5];
    sprintf(priorityString, "%d", priority);
    sprintf(samplingRateString, "%d", priority);

    /* if getEnvData returns -1 then the event will not be sent */
    if (getEnvData(samplingRateString, priorityString, category) == -1) {
        return 0;
    }
    if ((strcmp(comm_type, "TCP") == 0) || (strcmp(comm_type, "EPOLL") == 0)) {
        sendInstRecordTCP(&record);
    }
//    else {
//		sendInstRecordUDP(&record);
//	}

    return 0;
}
/*
 * This is data access probe function.
 * This function recieves the typical tag to send and the 
 * address of the varaible to read.
 */
extern "C"
int accessDataInst(const char *args, const char *tag, void *varAdd)
{
    /*
     * procLimiter defines if this rank can send data
     */
    if (rank % procLimiter != 0)
        return 0;


    /* Keep a all args here. ProMon will send it one time to inform Analyzer what are the args */
    if (prgArgs == " ") {
        /* atExit will be called when the process exits. */
        atexit(atExit);
        /* Make sure no file kept open after terminating */
        signal(SIGINT, atExit);
        prgArgs = args;
        string temp = "1:1:PROMON:" + string(FIRST_RECORD) + ":" + prgArgs;
//		sendInstRecord(prgArgs.c_str(), temp.c_str());
    }



    /*
     * The tag that is passed to this function from the monitored application is in the format
     * SamplingRate:Priority:Category:Name:varType.
     * The event that will be sent is in this format
     * Name:varType:value
     */

    char tmp[BUF_SIZE];
    char *samplingRate, *priority, *category;
    char varValue[BUF_SIZE] = "";
    char *varType;
    char *strippedTag;

    sprintf(tmp, "%s", tag);

    samplingRate = strtok(tmp, ":"); //samplingRate
    priority = strtok(NULL, ":"); //priority
    category = strtok(NULL, ":"); //category
    strippedTag = strtok(NULL, ":"); //Name
    varType = strtok(NULL, ":"); //varType

    /* if getEnvData returns -1 then the event will not be sent */
    if (getEnvData(samplingRate, priority, category) == -1) {
        return 0;
    }

    if (strcmp(varType, "short") == 0) {
        short *temp = static_cast<short *> (varAdd);
        sprintf(varValue, "%hu", *temp);
    } else if (strcmp(varType, "int") == 0) {
        int *temp = static_cast<int *> (varAdd);
        sprintf(varValue, "%d", *temp);
    } else if (strcmp(varType, "long") == 0) {
        long *temp = static_cast<long *> (varAdd);
        sprintf(varValue, "%ld", *temp);
    } else if (strcmp(varType, "float") == 0) {
        float *temp = static_cast<float *> (varAdd);
        sprintf(varValue, "%g", *temp);
    } else if (strcmp(varType, "long double") == 0) {
        long double *temp = static_cast<long double *> (varAdd);
        sprintf(varValue, "%Lg", *temp);
    } else if (strcmp(varType, "double") == 0) {
        double *temp = static_cast<double *> (varAdd);
        sprintf(varValue, "%g", *temp);
    } else if (strcmp(varType, "bool") == 0) {
        short *temp = static_cast<short *> (varAdd);
        sprintf(varValue, "%s", (*temp ? "TRUE" : "FALSE"));
    }


    string fullTag(strippedTag);
    fullTag += string(":") + string(varValue);

    /* Let's send the data now */
    if ((strcmp(comm_type, "TCP") == 0) || (strcmp(comm_type, "EPOLL") == 0)) {
        sendInstRecordTCP(fullTag.c_str());
    } else if (strcmp(comm_type, "MSGQUEUE") == 0) {
        sendInstRecordMSGQUEUE(fullTag.c_str());
    } else if (strcmp(comm_type, "POSIX") == 0) {
        sendInstRecordPOSIX(fullTag.c_str());
    } else {
        sendInstRecordUDP(fullTag.c_str());
    }

    return 0;
}


