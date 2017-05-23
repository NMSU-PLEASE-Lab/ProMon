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
using namespace std;

class serverEPoll : public server {
public:
    serverEPoll(int p);
    virtual ~serverEPoll();
    int start();
private:
     void handleBuffer(int descriptor, const char* buffer);
     int make_socket_non_blocking (int sfd);
     int create_and_bind (char *port);

     /*
      * Map each received message to its descriptor.
      * The reason we do this is to handle the partial messages.
      */
     map<int, string> desRecords;

     /*
      * Keep the rank and jobid for each connection in the rank:jobid format.
      * Use this data to close any open file by analyzer after connection closes or breaks.
      */
     map<int, string> rank_jobids;
};

#endif /*SERVEREPOLL_H*/

