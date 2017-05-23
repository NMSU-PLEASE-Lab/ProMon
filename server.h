#ifndef SERVER_H
#define SERVER_H

using namespace std;

/* Let the compiler know that such class exists */

class server {
public:
    server(int p);
    virtual ~server();
    virtual int start() = 0;
    int getport();
    void setport(int p);
protected:
    int port; // The communication port
};

#endif	/* server_H */

