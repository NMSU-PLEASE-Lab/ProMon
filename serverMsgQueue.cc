#include "serverMsgQueue.h"

using namespace std;

/*
 * This number is not any limit. 
 * Linux increases the max events by need.
 */
#define MAXEVENTS 200
#define MSGSZ     1024


serverMsgQueue::serverMsgQueue(int p) : server(p)
{

}

serverMsgQueue::~serverMsgQueue()
{

}


/* The main function to serve IPC - Message Queue
*/
int serverMsgQueue::start()
{
    int msqid;
    key_t key;
    /*
    * The received event is the form of msgpack object,
    * msgpack reference: https://github.com/msgpack/msgpack-c
    */
    /* Reserve msgpack unpacker buffer */
    m_pac.reserve_buffer(MSG_PACK_BUFFER);
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
     * Receive the events
     */
    ssize_t ret = msgrcv(msqid, m_pac.buffer(), m_pac.buffer_capacity(), 1, 0);
    while (ret >= 0){
        m_pac.buffer_consumed((size_t) ret);
/* Parse events in the form of msgpack object, one by one
                     * And pass to buffer handler
                     */
        msgpack::object_handle oh;
        while (m_pac.next(oh)) {
            msgpack::object msg = oh.get();
            /*
            * MSGPACK Events are send to analyzer for separating individual events.
            */
            Analyzer::handleMSG_static(msg);
        }
        ret = msgrcv(msqid, m_pac.buffer(), m_pac.buffer_capacity(), 1, 0);
    }

    return EXIT_SUCCESS;
}

