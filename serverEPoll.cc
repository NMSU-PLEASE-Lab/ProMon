/* This a customized code from https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c */
#include "serverEPoll.h"

using namespace std;

/*
 * This number is not any limit. 
 * Linux increases the max events by need.
 */
#define MAXEVENTS 200

serverEPoll::serverEPoll(int p) : server(p)
{

}

serverEPoll::~serverEPoll()
{

}


/*
 * This function creates and binds a TCP socket
 */
int serverEPoll::make_socket_non_blocking(int sfd)
{
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL fcntl");
        return -1;
    }

    return 0;
}

/*
 * This function uses a standard code block for a portable way of getting a IPv4 or IPv6 socket. 
 */
int serverEPoll::create_and_bind(char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL getaddrinfo: %s", gai_strerror(s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            /* We managed to bind successfully! */
            break;
        }

        close(sfd);
    }

    if (rp == NULL) {
        ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL Could not bind!");
        return -1;
    }

    freeaddrinfo(result);

    return sfd;
}

/*
 * The main function to serve epoll
 */
int serverEPoll::start()
{
    int sfd, s;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;
    char portTemp[BUF_SIZE];

    sprintf(portTemp, "%d", port);

    sfd = create_and_bind(portTemp);
    if (sfd == -1)
        abort();

    s = make_socket_non_blocking(sfd);
    if (s == -1)
        abort();

    s = listen(sfd, SOMAXCONN);
    if (s == -1) {
        ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL listen!");
        abort();
    }

    efd = epoll_create(1); //input>0 based on the documentation of epoll
    if (efd == -1) {
        ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL epoll_create!");
        abort();
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1) {
        ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL epoll_ctl!");
        abort();
    }

    /* Buffer where events are returned */
    events = (epoll_event *) calloc(MAXEVENTS, sizeof(event));

    /* The event loop */
    while (1) {
        int n, i;

        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                /* An error has occured on this fd, or the socket is not
                   ready for reading (why were we notified then?) */
                ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL epoll error!");
                close(events[i].data.fd);
                continue;
            } else if (sfd == events[i].data.fd) {
                /* We have a notification on the listening socket, which
                   means one or more incoming connections. */
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof(in_addr);
                    infd = accept(sfd, &in_addr, &in_len);
                    if (infd == -1) {
                        if ((errno == EAGAIN) ||
                            (errno == EWOULDBLOCK)) {
                            /* We have processed all incoming
                               connections. */
                            break;
                        } else {
                            ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL accept!");
                            break;
                        }
                    }

                    s = getnameinfo(&in_addr, in_len,
                                    hbuf, sizeof(hbuf),
                                    sbuf, sizeof(sbuf),
                                    NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0) {
                        ProMon_logger(PROMON_DEBUG, "ProMon SERVEREPOLL: Accepted connection on descriptor %d "
                            "(host=%s, port=%s)", infd, hbuf, sbuf);
                    }

                    /* Make the incoming socket non-blocking and add it to the
                       list of fds to monitor. */
                    s = make_socket_non_blocking(infd);
                    if (s == -1)
                        abort();

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1) {
                        ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL epoll_ctl!");
                        abort();
                    }
                }
                continue;
            } else {
                /* We have data on the fd waiting to be read. Read and
                   display it. We must read whatever data is available
                   completely, as we are running in edge-triggered mode
                   and won't get a notification again for the same
                   data. */
                int done = 0;

                while (1) {

                    /*
                    * The received event is the form of msgpack object,
                    * msgpack reference: https://github.com/msgpack/msgpack-c
                    */
                    /* Reserve msgpack unpacker buffer */
                    m_pac.reserve_buffer(MSG_PACK_BUFFER);

                    /*
                     * Receive events in the form of msgpack
                     */
                    ssize_t count = read(events[i].data.fd, m_pac.buffer(), m_pac.buffer_capacity());

                    if (count == 0) {
                        /* End of file. The remote has closed the
                           connection. */
                        done = 1;
                        break;
                    } else if (count == -1) {

                        /* If errno == EAGAIN, that means we have read all
                     data. So go back to the main loop. */
                        if (errno != EAGAIN) {
                            ProMon_logger(PROMON_ERROR, "ProMon SERVEREPOLL  read!");
                            done = 1;
                        }
                        break;
                    }
                    m_pac.buffer_consumed((size_t) count);

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
                        if (rank_jobids.find(events[i].data.fd) == rank_jobids.end()) {
                            int rank;
                            string job_id;
                            msg.via.array.ptr[2].convert(rank);//access rank from its msgpack object
                            msg.via.array.ptr[3].convert(job_id);//access jobid from its msgpack object

                            char rankStr[21];
                            sprintf(rankStr, "%d", rank);
                            rank_jobids[events[i].data.fd] = rankStr + string(":") + job_id;
                        }

                    }

                }


                if (done) {
                    ProMon_logger(PROMON_DEBUG, "ProMon SERVEREPOLL: Closed connection on descriptor %d",
                                  events[i].data.fd);

                    /* Closing the descriptor will make epoll remove it
                       from the set of descriptors which are monitored. */
                    close(events[i].data.fd);

                    /* Clean up resources in analyzer */
                    string temp = rank_jobids[events[i].data.fd];
                    char *buf = new char[temp.length() + 1];
                    strcpy(buf, temp.c_str());
                    char *rank = strtok(buf, ":");
                    char *job_id = strtok(NULL, ":");
                    ProMon_logger(PROMON_DEBUG, "ProMon SERVEREPOLL: Cleaning resources for %s:%s!", job_id, rank);
                    Analyzer::sumup_clean_static(rank, job_id);
                    rank_jobids.erase(events[i].data.fd);
                }
            }
        }
    }

    free(events);

    close(sfd);

    return EXIT_SUCCESS;
}

