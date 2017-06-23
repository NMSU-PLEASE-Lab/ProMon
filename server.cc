#include "server.h"

using namespace std;

/*
 Construct  the communication object with the the type of the communication and (p) the port
 */
server::server(int p) {
    port = p;
}

server::~server() {
    port = 0;
}

int server::getport() {
    return port;
}

void server::setport(int p) {
    port = p;
}
