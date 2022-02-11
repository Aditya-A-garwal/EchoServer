#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket_utils.hpp"

#define PORT                    8080                    // port to listen on for client connection requests
#define MAX_EVENTS              65535                   // maximum number of events to process in a single loop block

#define DELIM_CHAR              (char) 0                // delimiter character of message
#define MSG_BLOCK_LEN           256                     // buffer size while reading in messages

int
__echo (simple_callback::callback * cb, const int & epoll_fd, const sockaddr_in & address, const int & addrlen)
{

    // get the file descriptor of the event's socket and message string for the socket on which a read event has opened
    const int32_t &                 sd = cb->sock;
    simple_string::string_buffer &  s  = cb->s;

    size_t                          num_recv = 0;
    char                            buff[MSG_BLOCK_LEN];

    num_recv                                 = recv (sd, buff, MSG_BLOCK_LEN, 0);

    if (num_recv == 0) {                                // if no characters were received, this was a connection being closed

        // get the peername and print it out
        getpeername (sd, (struct sockaddr *) &address, (socklen_t *) &address);
        printf ("Host disconnected, ip %s, port %d \n", inet_ntoa (address.sin_addr), ntohs (address.sin_port));

        // cleanup by removing this socket file descriptor from the epoll instance, closing the socket and deleting the callback
        epoll_ctl (epoll_fd, EPOLL_CTL_DEL, sd, NULL);
        close (sd);
        delete cb;
    } else {                                            // a message has been received
        s.append (buff, num_recv);
        if (buff[num_recv - 1] == DELIM_CHAR) {         // if delemiter has been received, echo back the message
            printf ("Sending to %d: %s\n", sd, s.get ());
            send (sd, s.get (), s.len, 0);
            s.reset ();
        }
    }

    return 1;
}

int
__accept (simple_callback::callback * cb, const int & epoll_fd, const sockaddr_in & address, const int & addrlen)
{

    // get the file descriptor of the event's socket and accept a new connection on a new socket
    const int & sd         = cb->sock;
    int         new_socket = accept (sd, (struct sockaddr *) &address, (socklen_t *) &addrlen);

    printf ("New connection, socket %d, ip %s, port %d\n", new_socket, inet_ntoa (address.sin_addr),
            ntohs (address.sin_port));

    // create a new callback object for the accepted connection and register it with the epoll instance
    epoll_event secondary_event;
    secondary_event.data.ptr = (void *) (new simple_callback::callback (new_socket, &__echo));
    secondary_event.events   = EPOLLIN;

    epoll_ctl (epoll_fd, EPOLL_CTL_ADD, new_socket, &secondary_event);  // register the event

    return 1;
}

int
main (int argc, char * argv[])
{
    int32_t     listener;                               // file descriptor of socket which listens for connections

    int32_t     epoll_fd;                               // file descriptor of epoll instance

    sockaddr_in address;                                // address of the server
    u_int32_t   addrlen;                                // length of the address struct
    epoll_event primary_event;                          // struct to store the primary event (new connection request)
    epoll_event events[MAX_EVENTS];                     // buffer to store events on connection listener and client sockets

    listener                = socket (AF_INET, SOCK_STREAM, 0); // create a socket and set its options

    // int opt = 1;
    // setsockopt(listener, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, 4);

    address.sin_family      = AF_INET;                  // internet protocol
    address.sin_port        = htons (PORT);             // convert host port to network address
    address.sin_addr.s_addr = INADDR_ANY;
    // inet_pton(AF_INET, "192.168.1.22", &address.sin_addr);

    // bind the listener socket to the server's address and start listening
    bind (listener, (sockaddr *) &address, sizeof (address));
    listen (listener, 65535);                           // 65535 connections will be queued before all others are refused
    printf ("Listener on port %d \nWaiting for connections\n", PORT);

    // create an epoll instance along with a callback object
    epoll_fd = epoll_create1 (0);
    simple_callback::callback cb1 (listener, &__accept);// call the accept function in case an event happens on the primary socket
    primary_event.data.ptr = (void *) &cb1;
    primary_event.events   = EPOLLIN;                   // the event should be a read event

    epoll_ctl (epoll_fd, EPOLL_CTL_ADD, listener, &primary_event); // register the event

    while (1) {
        int32_t num_events = epoll_wait (epoll_fd, events, MAX_EVENTS, -1); // get the events (wait indefinitely (-1) if there are no events)
        for (int i = 0; i < num_events; i++) {
            simple_callback::callback * cb = (simple_callback::callback *) (events[i].data.ptr);
            cb->func (cb, epoll_fd, address, addrlen);
        }
    }

    // close the listener and epoll instances
    close (epoll_fd);
    close (listener);

    return 0;
}
