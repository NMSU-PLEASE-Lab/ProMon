#include "serverPOSIX.h"

using namespace std;

/*
 * This number is not any limit. 
 * Linux increases the max events by need.
 */
#define MAXEVENTS 200
#define MSGSZ     1024



/*
 * Map each received message to its descriptor.
 * The reason we do this is to handle the partial messages.
 */
//static map<int, string> desRecords;

serverPOSIX::serverPOSIX(int p) : server(p)
{

}

serverPOSIX::~serverPOSIX()
{

}

/* The main function to serve IPC - Message Queue
 */
int serverPOSIX::start()
{
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
    mqd_t msqid = mq_open("PROMON_Queue", O_RDONLY);

    if (msqid == -1) {
        ProMon_logger(PROMON_DEBUG, "Error opening the POSIX QUEUE");
    }



    /*
     * Receive the events
     */
    ssize_t ret = mq_receive(msqid, m_pac.buffer(), m_pac.buffer_capacity(), 0);
    while (ret != -1) {
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
        ret = mq_receive(msqid, m_pac.buffer(), m_pac.buffer_capacity(), 0);
    }
    return EXIT_SUCCESS;
}

