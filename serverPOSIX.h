#ifndef SERVERPOSIX_H
#define SERVERPOSIX_H
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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "predefs.h"
#include "analyzer.h"
#include "util.h"
#include <mqueue.h>
#include "server.h"

using namespace std;

/* Let compiler know that these classes exist. */
class Analyzer;
 
class serverPOSIX : public server {
public:
    serverPOSIX(int p);
    virtual ~serverPOSIX();
    int start();
private:
     //void handleBuffer(int descriptor, const char* buffer);
    void handleBuffer(const char* buffer);
//     int make_socket_non_blocking (int sfd);
//     int create_and_bind (char *port);
     map<int, string> desRecords;
};

#endif /*SERVERMSGQUEUE_H*/

