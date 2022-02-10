#include <cstring>
#include <cstdio>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket_utils.hpp"

#define DELIM_CHAR (char) 0
#define PORT 8080
#define MSG_BLOCK_LEN 256

int
main (int argc, char const * argv[])
{
    long                         sock;
    simple_string::string_buffer buff;
    sockaddr_in                  serv_addr, clie_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons (8080);
    inet_pton (AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    printf ("Attempting to create socket");
    sock = socket (AF_INET, SOCK_STREAM, 0);
    printf ("\rSuccessfully created socket\n");

    // bind( sock, (sockaddr *)&clie_addr, sizeof( clie_addr ) );

    printf ("Attempting to connect to server");
    while (connect (sock, (sockaddr *) &serv_addr, sizeof (serv_addr)) == 0)
        continue;
    printf ("\rSuccesfully connected to server\n");

    while (1) {
        printf (">> ");
        buff.reset ();
        char c;
        do {
            c = getchar ();
            buff.append (c);
        } while (c != '\n');

        buff[buff.len - 1] = DELIM_CHAR;

        if (buff[0] == 'q' && buff.len == 2)
            break;
        send (sock, buff.get (), buff.len, 0);

        buff.reset ();

        int  num_recv_tot = 0, num_recv;
        char temp_buff[MSG_BLOCK_LEN];

        while (1) {
            num_recv = recv (sock, temp_buff, MSG_BLOCK_LEN, 0);
            num_recv_tot += num_recv;
            if (temp_buff[num_recv - 1] == DELIM_CHAR)
                break;
            buff.append (temp_buff, num_recv);
        }
        if (num_recv > 1)
            buff.append (temp_buff, num_recv - 1);

        printf ("%lu\t%s\n", (unsigned long) buff.len, buff.get ());
    }

    printf ("CLOSING CONNECTION\n");
    close (sock);

    return 0;
}
