#ifndef SERVERTCP_H
#define	SERVERTCP_H

#include <stdio.h>
#include <stdlib.h>   /* needed for os x */
#include <string.h>   /* for memset */
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
#include <string>
#include <map>
#include <msgpack.h>
#include <msgpack.hpp>
#include "server.h"

using namespace std;

class serverTCP : public server {
public:
    serverTCP(int p);
    virtual ~serverTCP();
    int start();

private: 
     /*
      * Keep the rank and jobid for each connection in the rank:jobid format.
      * Use this data to close any open file by analyzer after connection closes or breaks.
      */
     map<int, string> rank_jobids;

    /* Unpacker object for unpacking msgpack buffer */
    msgpack::unpacker m_pac;

};

#endif /*SERVERTCP_H*/

