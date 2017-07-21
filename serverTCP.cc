#include "serverTCP.h"

using namespace std;

serverTCP::serverTCP(int p) : server(p)
{

}

serverTCP::~serverTCP()
{

}

/*
 * This function creates a TCP server socket to receive all instrumentation records.
 */
int serverTCP::start()
{
    struct sockaddr_in si_me, si_other;
    int s, newsockfd, i, slen = sizeof(si_other);

    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: Could not get TCP socket!");


    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me)) == -1)
        ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: Could not bind!");

    /*
     * The second parameter is the backlog that defines the maximum length for the
     * queue of pending connections for the socket
     */
    listen(s, TCP_QUEUE_SIZE);

    i = 0;

    /*
    * The received event is the form of msgpack object,
    * msgpack reference: https://github.com/msgpack/msgpack-c
    */
    /* Reserve msgpack unpacker buffer */
    m_pac.reserve_buffer(MSG_PACK_BUFFER);
    for (;;) {
        /*
         * accept connection from client.
         */
        newsockfd = accept(s, (struct sockaddr *) &si_other, (socklen_t *) &slen);
        if (newsockfd < -1)
            ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: accept error!!! %d", errno);

        int childpid = fork();
        if (childpid < 0)
            ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: fork error!!! %d", errno);

        if (childpid == 0) {
            /*
             * The exit is called when the process ends
             */
            string strRecords("");
            vector<string> splitRecs;
            while (1) {
                /*
                 * Each node is handled by a child process.
                 */
                //printf("\nchild %d\n", childpid);
                ssize_t ret = recv(newsockfd, m_pac.buffer(), m_pac.buffer_capacity(), MSG_WAITALL);
                if (ret == -1) {
                    ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: Broken connection!!! %d", errno);
                    //handlingEndOfApplicaitonRun
                    close(newsockfd);
                    _Exit(3); //child exits
                    exit(0);
                }

                else if(ret == 0) {
                    ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: Closed connection on descriptor %d", newsockfd);
                    /* Clean up resources in Analyzer */
                    string temp = rank_jobids[newsockfd];
                    char *buf = new char[temp.length() + 1];
                    strcpy(buf, temp.c_str());
                    char *rank = strtok(buf, ":");
                    char *job_id = strtok(NULL, ":");
                    ProMon_logger(PROMON_DEBUG, "ProMon SERVERTCP: Cleaning resources for %s:%s!", job_id, rank);
                    Analyzer::sumup_clean_static(rank, job_id);
                    rank_jobids.erase(newsockfd);

                    //handlingEndOfApplicaitonRun
                    close(newsockfd);
                    _Exit(3); //child exits
                    exit(0);
                }
                else {
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

                        /* Keep the rank and job id to clean up in case the monitored application crashes. */
                        if (rank_jobids.find(newsockfd) == rank_jobids.end()) {
                            int rank;
                            string job_id;
                            msg.via.array.ptr[2].convert(rank);//access rank from its msgpack object
                            msg.via.array.ptr[3].convert(job_id);//access jobid from its msgpack object

                            char rankStr[21];
                            sprintf(rankStr, "%d", rank);
                            rank_jobids[newsockfd] = rankStr + string(":") + job_id;
                        }

                    }
                }

                i++;
            }
        } else {
            /*
             * close the copy of child with parent
             */
            close(newsockfd);
        }

    }
    close(s);
    return 0;
}

