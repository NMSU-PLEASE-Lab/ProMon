#ifndef SERVERUDP_H
#define SERVERUDP_H

#include <stdio.h>
#include <stdlib.h>   /* needed for os x */
#include <string.h>   /* for memset */
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>   /* for printing an Internet address in a user-friendly way */
#include <netinet/in.h>
#include <errno.h>   /* defines ERESTART, EINTR */
#include <unistd.h>
#include <stdint.h>
#include <iostream>
#include <list>
#include <sys/stat.h>
#include <fstream>
#include "predefs.h"
#include "analyzer.h"
#include "util.h"
#include "server.h"
#include <pthread.h>
#include <map>
#include <msgpack.h>
#include <msgpack.hpp>

using namespace std;

class serverUDP : public server {
public:
    serverUDP(int p);
    virtual ~serverUDP();
    int start();
    void cleanup();
private:
    pthread_t thread ;
    void startPeriodicCleanup();
    void stopPeriodicCleanup();
    /*
     * Keep the rank and jobid for each connection in the rank:jobid format.
     * Use this data to close any open file by analyzer after connection closes or breaks.
     */
    map<string, long> rank_jobids;
    int maxInactivity;

    /* Unpacker object for unpacking msgpack buffer */
    msgpack::unpacker m_pac;
};

void *TimerRoutine(void *);
#endif /*SERVERUDP_H*/
