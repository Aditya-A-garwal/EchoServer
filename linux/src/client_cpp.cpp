#include <cstring>
#include <cstdio>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket_utils.hpp"

#define DELIM_CHAR (char) 0                                 // Delimiting character in client messages
#define PORT 8080                                           // Port which server is listening on for connections
#define MSG_BLOCK_LEN 256                                   // Buffer size while reading in messages

int
main (int argc, char const * argv[])
{
    int32_t                      sock;                      // socket file descriptor
    simple_string::string_buffer msg;                       // resizeable string to store message in
    sockaddr_in                  clie_addr;                 // structs for storing addresses of client
    sockaddr_in                  serv_addr;                 // structs for storing addresses of server

    serv_addr.sin_family = AF_INET;                         // Internet protocol
    serv_addr.sin_port   = htons (PORT);                    // convert port to network address
    inet_pton (AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    printf ("Attempting to create socket");
    sock = socket (AF_INET, SOCK_STREAM, 0);                // get socket file descriptor
    printf ("\rSuccessfully created socket\n");

    // bind( sock, (sockaddr *)&clie_addr, sizeof( clie_addr ) );

    printf ("Attempting to connect to server");
    while (connect (sock, (sockaddr *) &serv_addr, sizeof (serv_addr)) == 0)
        continue;
    printf ("\rSuccesfully connected to server\n");

    while (1) {

        // reset the message string and print the prompt
        printf (">> ");
        msg.reset ();
        char c;

        // read in user input from stdin
        do {
            c = getchar ();
            msg.append (c);
        } while (c != '\n');

        // end the message with the delimiter and check if the user wants to quit
        msg[msg.len - 1] = DELIM_CHAR;
        if (msg[0] == 'q' && msg.len == 2)
            break;
        send (sock, msg.get (), msg.len, 0);

        msg.reset ();

        size_t num_recv_tot = 0;
        size_t num_recv;
        char   temp_buff[MSG_BLOCK_LEN];

        while (1) {
            num_recv = recv (sock, temp_buff, MSG_BLOCK_LEN,
                             0);                         // read MSG_BLOCK_LEN characters into the buffer
            num_recv_tot += num_recv;                    // update total character recieved

            // if delimiting character has been recieved, message has been recieved
            if (temp_buff[num_recv - 1] == DELIM_CHAR)
                break;

            // append buffer to message string
            msg.append (temp_buff, num_recv);
        }
        if (num_recv > 1)
            msg.append (temp_buff, num_recv - 1);

        printf ("%ul\t%s\n", (u_int32_t) msg.len, msg.get ());
    }

    // since the user has quit, close the connection
    printf ("CLOSING CONNECTION\n");
    close (sock);

    return 0;
}
