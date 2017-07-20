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
 * The length of job_id, needed for msgpack object
 */
static size_t jobIdLength = 0;

/*
 * The user name that is running this job
 */
static char *userName = NULL;

/*
 * The length of userName, needed for msgpack object
 */
static size_t userNameLength = 0;

/*
 * The job management system that is running this job
 */
const static char *jobMS = NULL;

/*
 * The length of jobMS, needed for msgpack object
 */
static size_t jobMSLength = 0;

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
static socklen_t slen = sizeof(si_other);

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
 * Event fields for begining and end of events
 */

static char const *start_or_end_record_name = "RECORD-PROGRAMMABLE";
static char const *start_or_end_record_type = "DUAL";
static char const *start_or_end_record_category = "Application";
static unsigned int start_or_end_record_priority = 3;
static unsigned int start_or_end_record_sampling_rate = 1;
static unsigned int start_record_position = 0;
static unsigned int end_record_position = 1;


/*
 * Called when the application exit execution. Using atexit function.
 * Send the last event and close the connection.
 * maximum 32 function can register with atexit()
 */
void atExit()
{
    string temp = "1:1:PROMON:" + string(LAST_RECORD);
    sendInstRecord(prgArgs.c_str(),start_or_end_record_name,start_or_end_record_type,
                   start_or_end_record_category,start_or_end_record_priority,
                   end_record_position,start_or_end_record_sampling_rate);
    closeSocket();
}

/*
 * Similar to above but for Ctrl+C exit. Handling signals.
 */
void atExit(int sigNum)
{
    string temp = "1:1:PROMON:" + string(LAST_RECORD);
    sendInstRecord(prgArgs.c_str(),start_or_end_record_name,start_or_end_record_type,
                   start_or_end_record_category,start_or_end_record_priority,
                   end_record_position,start_or_end_record_sampling_rate);
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
    sendInstRecord(prgArgs.c_str(),start_or_end_record_name,start_or_end_record_type,
                   start_or_end_record_category,start_or_end_record_priority,
                   end_record_position,start_or_end_record_sampling_rate);
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
    userNameLength = strlen(userName);
    jobMS = getenv("JOB_SYSTEM");
    comm_type = getenv("COMM_TYPE");
    if (comm_type == NULL) {
        fprintf(stderr, "ProMon ProbLib: Communication protocol is not set !!!\n\n");
        exit(1);
    }

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
    jobMSLength = strlen(jobMS);
    jobIdLength = strlen(job_id);


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
 * TEMPORARILY DISABLED
 */
//static int getEnvData(const char *eventRate, const char *eventPriority, const char *eventCategory)
//{
//    char *rate = getenv("PROMON_SAMPLING_RATE");
//    char *priority = getenv("PROMON_BLOCK_PRIORITY");
//    char *category = getenv("PROMON_BLOCK_CATEGORY");
//
//
//    if (rate == NULL && priority == NULL && category == NULL)
//        return 0;
//
//    /* Priorities are from 0 to 9 only */
//    if (priority != NULL) {
//        if (strstr(priority, eventPriority) != NULL) //strstr parameters is not how are used to (it is in reverse)!!!!
//            return -1;
//    }
//
//    if (category != NULL) {
//        if (strstr(category, eventCategory) != NULL)
//            return -1;
//    }
//    /* No issue then let the event dispatched.
//     * return 0 is the ok from getEnvData function
//     */
//    return 0;
//}

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
int sendInstRecordTCP(msgpack_sbuffer* msgpack_buffer)
{

    /*
     * Check if we have created the socket; if not, create one.
     * We really don't want to create socket with generating each instrumentation record.
     */
    if (s == -1) {
        openSocketTCP(IP, PORT);
    }

    /*
     * We count all events send.
     */
    eventCount++;
    /*
     * Send the events as message pack
     * In msgpack, data is the content of buffer and size is the total bytes of buffer
     */
    ssize_t n = send(s, msgpack_buffer->data, msgpack_buffer->size, 0);

    if (n < 0)
        fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));

    return 0;
}

/*
 * Creates instrumentation record, send it to the LDMS using IPC - Message Queue socket
 * tag is the main data generated from the running code. It contains tag info and some other data from the application.
 * tag does not have the time that it generated. We create the time here in this function and send the data.
 */
int sendInstRecordMSGQUEUE(msgpack_sbuffer* msgpack_buffer)
{

    /*
     * We count all events generated.
     */
    eventCount++;


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

    /*
     * In msgpack, data is the content of buffer and size is the total bytes of buffer
    */
    strcpy(sbuf.mtext, msgpack_buffer->data);
    buf_length = msgpack_buffer->size + 1;

    /*
     * Send the event in the form of msgpack
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
int sendInstRecordPOSIX(msgpack_sbuffer* msgpack_buffer)
{
    /*
     * We count all events generated.
     */
    eventCount++;

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
    /*
     * Send the events as message pack
     * In msgpack, data is the content of buffer and size is the total bytes of buffer
     */
    if (mq_send(mqdes, msgpack_buffer->data, msgpack_buffer->size, 0) == -1) {
        fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));
    }
    return 0;
}

/*
 * Creates instrumentation record, send it to the analyzer using UDP
 * tag is the main data generated from the running code. It contains tag info and some other data from the application.
 * tag does not have the time that it generated. We create the time here in this function and send the data.
 */
int sendInstRecordUDP(msgpack_sbuffer* msgpack_buffer)
{

    /*
     * Check if we have created the socket; if not, create one.
     * We really don't want to create socket with generating each instrumentation record.
     */
    if (s == -1) {
        openSocketUDP(IP, PORT);
    }


    /*
     * We count all events generated.
     */
    eventCount++;

    ssize_t n = sendto(s, msgpack_buffer->data, msgpack_buffer->size, 0,
                   (struct sockaddr *) &si_other, slen);
    if (n < 0)
        fprintf(stderr, "ProMon ProbeLib: sending data failed! %s\n", strerror(errno));

    return 0;
}

/*
 * This function is for all events except DATA_ACCESS
 */
extern "C"
int sendInstRecord(const char *args, const char *name, const char *type, const char *category, unsigned int priority,
                   unsigned int position, unsigned int samplingRate)
{

    /*
     * procLimiter defines if this rank can send data
     */
    if (rank % procLimiter != 0)
        return -1;

    /* Keep a all args here. ProMon will send it one time to inform Analyzer what are the args */
    if (prgArgs == " ") {
        /* GET all env Data at START of first event */
        getEnvData();

        /* atExit will be called when the process exits. */
        atexit(atExit);

        /* Make sure no file kept open after terminating */
        signal(SIGINT, atExit);
        prgArgs = args;
        sendInstRecord(args,start_or_end_record_name,start_or_end_record_type,
                       start_or_end_record_category,start_or_end_record_priority,
                       start_record_position,start_or_end_record_sampling_rate);
    }

    /* This block is for sampling of events, need to work on this
     * since its not working now, this block is disabled for now
     */
//    {
//        char priorityString[5];
//        char samplingRateString[5];
//        sprintf(priorityString, "%d", priority);
//        sprintf(samplingRateString, "%d", priority);
//
//
//        if (getEnvData(samplingRateString, priorityString, category) == -1) {
//            return 0;
//        }
//    }
    /* To get the current second and nanoseconds*/
    struct timespec time1;
    clock_gettime(CLOCK_REALTIME, &time1);


    /*
    * create msgpack buffer and serializer instance.
    * REFERENCE: https://github.com/msgpack/msgpack-c
    */
    msgpack_sbuffer msgpack_buffer;
    msgpack_sbuffer_init(&msgpack_buffer);


    /* serialize event values into the msgpacker buffer using msgpack_sbuffer_write */
    msgpack_packer pk;
    msgpack_packer_init(&pk, &msgpack_buffer, msgpack_sbuffer_write);

    /*
     * PACK the event fields one by one in order:
     * timeSec,timeNanoSec,rank,jobId,username,jobMS,eventType,
     * eventName,eventPosition,eventCount,eventCategory
     * The total length of event is 11 for non DATA_ACCESS event
     */

    msgpack_pack_array(&pk, 11);

    msgpack_pack_long(&pk, time1.tv_sec);

    msgpack_pack_long(&pk, time1.tv_nsec);

    msgpack_pack_int(&pk, rank);

    msgpack_pack_str(&pk, jobIdLength);
    msgpack_pack_str_body(&pk, job_id, jobIdLength);

    msgpack_pack_str(&pk, userNameLength);
    msgpack_pack_str_body(&pk, userName, userNameLength);

    msgpack_pack_str(&pk, jobMSLength);
    msgpack_pack_str_body(&pk, jobMS, jobMSLength);

    size_t eventTypeLength = strlen(type);
    msgpack_pack_str(&pk, eventTypeLength);
    msgpack_pack_str_body(&pk, type, eventTypeLength);

    size_t eventNameLength = strlen(name);
    msgpack_pack_str(&pk, eventNameLength);
    msgpack_pack_str_body(&pk, name, eventNameLength);


    msgpack_pack_int(&pk, position);

    msgpack_pack_long(&pk, eventCount);

    size_t eventCategoryLength = strlen(category);
    msgpack_pack_str(&pk, eventCategoryLength);
    msgpack_pack_str_body(&pk, category, eventCategoryLength);



    if ((strcmp(comm_type, "TCP") == 0) || (strcmp(comm_type, "EPOLL") == 0)) {
        sendInstRecordTCP(&msgpack_buffer);
    }
    else if (strcmp(comm_type, "MSGQUEUE") == 0) {
        sendInstRecordMSGQUEUE(&msgpack_buffer);
    } else if (strcmp(comm_type, "POSIX") == 0) {
        sendInstRecordPOSIX(&msgpack_buffer);
    } else {
        sendInstRecordUDP(&msgpack_buffer);
    }
    msgpack_sbuffer_destroy(&msgpack_buffer);
    return 0;

}
/*
 * This is data access probe function.
 * This function recieves the typical tag to send and the
 * address of the varaible to read.
 */
extern "C"
int accessDataInst(const char *args, const char *name, const char *type, const char *category, unsigned int priority,unsigned int position, unsigned int samplingRate,const char *varType, void *varAdd)
{
    /*
     * procLimiter defines if this rank can send data
     */
    if (rank % procLimiter != 0)
        return -1;

    /* Keep a all args here. ProMon will send it one time to inform Analyzer what are the args */
    if (prgArgs == " ") {
        /* GET all env Data at START of first event */
        getEnvData();

        /* atExit will be called when the process exits. */
        atexit(atExit);

        /* Make sure no file kept open after terminating */
        signal(SIGINT, atExit);
        prgArgs = args;
        sendInstRecord(args,start_or_end_record_name,start_or_end_record_type,
                       start_or_end_record_category,start_or_end_record_priority,
                       start_record_position,start_or_end_record_sampling_rate);
    }
    /* To get the current second and nanoseconds*/
    struct timespec time1;
    clock_gettime(CLOCK_REALTIME, &time1);

    /* This block is for sampling of events, need to work on this
     * since its not working now, this block is disabled for now
     */
//    {
//        char priorityString[5];
//        char samplingRateString[5];
//        sprintf(priorityString, "%d", priority);
//        sprintf(samplingRateString, "%d", priority);
//
//
//        if (getEnvData(samplingRateString, priorityString, category) == -1) {
//            return 0;
//        }
//    }


    /*
    * create msgpack buffer and serializer instance.
    * REFERENCE: https://github.com/msgpack/msgpack-c
    */
    msgpack_sbuffer msgpack_buffer;
    msgpack_sbuffer_init(&msgpack_buffer);


    /* serialize event values into the msgpacker buffer using msgpack_sbuffer_write */
    msgpack_packer pk;
    msgpack_packer_init(&pk, &msgpack_buffer, msgpack_sbuffer_write);

    /*
     * PACK the event fields one by one in order:
     * timeSec,timeNanoSec,rank,jobId,username,jobMS,eventType,
     * eventName,eventPosition,eventCount,eventCategory,variable_value
     * The total length of event is 12 for  DATA_ACCESS event
     */

    msgpack_pack_array(&pk, 12);

    msgpack_pack_long(&pk, time1.tv_sec);

    msgpack_pack_long(&pk, time1.tv_nsec);

    msgpack_pack_int(&pk, rank);

    msgpack_pack_str(&pk, jobIdLength);
    msgpack_pack_str_body(&pk, job_id, jobIdLength);

    msgpack_pack_str(&pk, userNameLength);
    msgpack_pack_str_body(&pk, userName, userNameLength);

    msgpack_pack_str(&pk, jobMSLength);
    msgpack_pack_str_body(&pk, jobMS, jobMSLength);

    size_t eventTypeLength = strlen(type);
    msgpack_pack_str(&pk, eventTypeLength);
    msgpack_pack_str_body(&pk, type, eventTypeLength);

    size_t eventNameLength = strlen(name);
    msgpack_pack_str(&pk, eventNameLength);
    msgpack_pack_str_body(&pk, name, eventNameLength);


    msgpack_pack_int(&pk, position);

    msgpack_pack_long(&pk, eventCount);

    size_t eventCategoryLength = strlen(category);
    msgpack_pack_str(&pk, eventCategoryLength);
    msgpack_pack_str_body(&pk, category, eventCategoryLength);

    /* Check variable type and pack the variable accordingly */
    if (strcmp(varType, "short") == 0) {
        short *temp = static_cast<short*> (varAdd);
        msgpack_pack_short(&pk, *temp);
    } else if (strcmp(varType, "int") == 0) {
        int *temp = static_cast<int*> (varAdd);
        msgpack_pack_int(&pk, *temp);
    } else if (strcmp(varType, "long") == 0) {
        long *temp = static_cast<long*> (varAdd);
        msgpack_pack_long(&pk, *temp);
    } else if (strcmp(varType, "float") == 0) {
        float *temp = static_cast<float*> (varAdd);
        msgpack_pack_float(&pk, *temp);
    }else if (strcmp(varType, "long long") == 0) {
        long long *temp = static_cast<long long*> (varAdd);
        msgpack_pack_long_long(&pk, *temp);
    }
    else if (strcmp(varType, "double") == 0) {
        double *temp = static_cast<double*> (varAdd);
        msgpack_pack_double(&pk, *temp);
    } else if (strcmp(varType, "bool") == 0) {
        short *temp = static_cast<short*> (varAdd);
        msgpack_pack_short(&pk, *temp);
    }

    if ((strcmp(comm_type, "TCP") == 0) || (strcmp(comm_type, "EPOLL") == 0)) {
        sendInstRecordTCP(&msgpack_buffer);
    }
    else if (strcmp(comm_type, "MSGQUEUE") == 0) {
        sendInstRecordMSGQUEUE(&msgpack_buffer);
    } else if (strcmp(comm_type, "POSIX") == 0) {
        sendInstRecordPOSIX(&msgpack_buffer);
    } else {
        sendInstRecordUDP(&msgpack_buffer);
    }
    msgpack_sbuffer_destroy(&msgpack_buffer);
    return 0;
}


