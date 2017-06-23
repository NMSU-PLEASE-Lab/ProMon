#include "serverMsgQueue.h"
using namespace std;

/*
 * This number is not any limit. 
 * Linux increases the max events by need.
 */
#define MAXEVENTS 200
#define MSGSZ     1024

/*
 * Declare the message structure.
 */

typedef struct msgbuf1 {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf1;


/*
 * Map each received message to its descriptor.
 * The reason we do this is to handle the partial messages.
 */
//static map<int, string> desRecords;

serverMsgQueue::serverMsgQueue(int p):server(p) {

}

serverMsgQueue::~serverMsgQueue() {

}

/*
 * Events are buffered. We need to prepare them to be analayzed. 
 * Mainly we need to separate messages as them all buffered in array 
 * received.
*/
void serverMsgQueue::handleBuffer(const char* buffer)
{
   vector<string> splitRecs;

   /*
    * Make sure you handle all records. Including those that
    * partially in the buffer
    */
   //desRecords[descriptor] += buffer;
   string strRecords = buffer;
   //desRecords[descriptor];

   /*
    * Records should not have any new line character 
    */
//   string::size_type pos = 0; 
//   while ( ( pos = strRecords.find ("\n",pos) ) != string::npos )
//   {
//      strRecords.erase ( pos, 2 );
//   }
  
  
   /* split is defined on top */ 
   splitRecs = split(strRecords, " ");
   for(vector<string>::iterator it = splitRecs.begin(); it != splitRecs.end(); it++)
   {
        string temp = *it;
        /*
         * If | was not found then this is partial record. The rest are in the buffer. We clear strRecord by only having the partial record.
         * Assumption: All partial record will not end with | and they are always the last record in the buffer
         */
        temp[temp.size()-1]='\0';   
        strRecords.clear();

        ProMon_logger(PROMON_DEBUG, "ProMon SERVERMSGQUEUE: %s", temp.c_str());

        char *record = new char[temp.size()]; //we have set the last character to null (above)
        strcpy(record, temp.c_str());
        Analyzer::handleMSG_static(record, sizeof buffer);
        delete[] record;
   }
   //desRecords[descriptor] = strRecords;
}


 /* The main function to serve IPC - Message Queue
 */
int serverMsgQueue::start()
{
    int msqid;
    key_t key;
    message_buf1  buf;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 1978;

    if ((msqid = msgget(key, IPC_CREAT | 0666)) < 0) {
        perror("msgget");
        exit(1);
    }

    
    /*
     * Receive an answer of message type 1.
     */
    
    while (msgrcv(msqid, &buf, MSGSZ, 1, 0) >= 0)
    {
         printf("%s\n", buf.mtext);
         ProMon_logger(PROMON_DEBUG, "ProMon GetPacket: %s", buf.mtext);
         handleBuffer(buf.mtext);
         //handleBuffer(events[i].data.fd, buf);
         
    }
    
        // handleBuffer(events[i].data.fd, buf);
   

//   if (done)
//   {
//        ProMon_logger(PROMON_DEBUG, "ProMon SERVEREPOLL: Closed connection on descriptor %d\n",
//             events[i].data.fd);
//
//       /* Closing the descriptor will make epoll remove it
//          from the set of descriptors which are monitored. */
//       close (events[i].data.fd);
//       /* delete the recrod related to this descriptor to handle partial messages */  
//       desRecords.erase(events[i].data.fd);
//    }
   

   return EXIT_SUCCESS;
}

