#ifndef SERVEREPOLL_H
#define SERVEREPOLL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <vector>
#include "predefs.h"
#include "analyzer.h"
#include "util.h"
#include "server.h"
#include <msgpack.h>
#include <msgpack.hpp>


using namespace std;

class serverEPoll : public server {
public:
    typedef msgpack::unique_ptr<msgpack::zone> unique_zone;
    serverEPoll(int p);
    virtual ~serverEPoll();
    int start();

private:
    /* Unpacker object for unpacking msgpack buffer */
    msgpack::unpacker m_pac;
    void handleBuffer(msgpack::object msg);
    int make_socket_non_blocking (int sfd);
    int create_and_bind (char *port);
     /*
      * Keep the rank and jobid for each connection in the rank:jobid format.
      * Use this data to close any open file by analyzer after connection closes or breaks.
      */
     map<int, string> rank_jobids;
};

#endif /*SERVEREPOLL_H*/

