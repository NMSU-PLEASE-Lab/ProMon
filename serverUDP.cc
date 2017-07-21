#include "serverUDP.h"

using namespace std;

serverUDP::serverUDP(int p) : server(p)
{
    maxInactivity = PROMON_UDP_INACTIVITY_PERIOD;
}

serverUDP::~serverUDP()
{

}

/*
 * This function creates a server UDP socket to receive all instrumentation records.
 */
int serverUDP::start()
{
    char *temp = getenv("PROMON_UDP_INACTIVITY_PERIOD");
    if (temp != NULL)
        maxInactivity = atoi(temp);

    /* In case that a monitored application crashes, this function frees the acquired resources */
    startPeriodicCleanup();

    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        ProMon_logger(PROMON_ERROR, "ProMon SERVERUDP: Could not get socket! ");


    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me)) == -1)
        ProMon_logger(PROMON_ERROR, "ProMon SERVERUDP: Could not bind!");

    /* keep track of all records in the map for serverUDP */
    struct timespec time1;

    i = 0;
    /*
    * The received event is the form of msgpack object,
    * msgpack reference: https://github.com/msgpack/msgpack-c
    */
    m_pac.reserve_buffer(MSG_PACK_BUFFER);

    for (;;) {
        clock_gettime(CLOCK_REALTIME, &time1);
        ssize_t ret = recvfrom(s, m_pac.buffer(), m_pac.buffer_capacity(), 0, (struct sockaddr *) &si_other,
                               (socklen_t *) &slen);
        if (ret == -1)
            ProMon_logger(PROMON_ERROR, "ProMon SERVERUDP: recvfrom error!!! %d", errno);
        else  {
            i++;
            ProMon_logger(PROMON_DEBUG, "ProMon SERVERUDP: Received packet from %s:%d",
                          inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));

            m_pac.buffer_consumed((size_t) ret);
            /* Parse events in the form of msgpack object, one by one
             * And pass to buffer handler
             */
            msgpack::object_handle oh;
            while (m_pac.next(oh)) {
                msgpack::object msg = oh.get();


                /* Keep the rank and job id to clean up in case the monitored application crashes. */
                int rank;
                string job_id;
                msg.via.array.ptr[2].convert(rank);//access rank from its msgpack object
                msg.via.array.ptr[3].convert(job_id);//access jobid from its msgpack object

                char rankStr[21];
                sprintf(rankStr, "%d", rank);
                rank_jobids[string(rankStr) + ":" + job_id] = time1.tv_sec;
                /*
                 * MSGPACK Events are send to analyzer for separating individual events.
                 */
                Analyzer::handleMSG_static(msg);

            }

        }

    }
    close(s);

    /* No more periodic check */
    stopPeriodicCleanup();

    return 0;
}

/*
 * In UDP, the servers is not aware if a monitored application stopped or crashed.
 * We have to periodically check and if there are such cases, we release the acquired resources.
 */
void serverUDP::startPeriodicCleanup()
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&thread, &attr, TimerRoutine, this);
}

void serverUDP::stopPeriodicCleanup()
{
    pthread_cancel(thread);
    pthread_join(thread, NULL);
}

void serverUDP::cleanup()
{
    struct timespec time1;
    clock_gettime(CLOCK_REALTIME, &time1);

    map<string, long>::iterator it;
    for (it = rank_jobids.begin(); it != rank_jobids.end(); it++) {
        if ((time1.tv_sec - it->second) > maxInactivity) {
            string temp = it->first;
            char *buf = new char[temp.length() + 1];
            strcpy(buf, temp.c_str());
            char *rank = strtok(buf, ":");
            char *job_id = strtok(NULL, ":");
            ProMon_logger(PROMON_DEBUG, "ProMon SERVERUDP: Cleaning resources for %s:%s!", job_id, rank);
            Analyzer::sumup_clean_static(rank, job_id);
            rank_jobids.erase(it);
        }
    }
}

/*
 * Performs the process of cleaning and releasing resources.
 * The defined map for serverUDP, keeps track of time for each value it contains.
 * if a record was not change for the period of time that is defined in the 
 * ProMon.cfg, then probably the related application is not running anymore
 * and delete the record from the map 
 */
void *TimerRoutine(void *arg)
{
    serverUDP *server = static_cast<serverUDP *> (arg);
    pthread_detach(pthread_self());

    int sweepTime = PROMON_UDP_PERIODIC_SWEEP;
    char *temp = getenv("PROMON_UDP_PERIODIC_SWEEP");
    if (temp != NULL)
        sweepTime = atoi(temp);

    while (true) {
        ProMon_logger(PROMON_DEBUG, "ProMon SERVERUDP: Check and sweep!");
        sleep(sweepTime);
        server->cleanup();
        pthread_testcancel();
    }
    return arg;
}
