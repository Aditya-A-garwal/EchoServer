#include <cstring>
#include <cstdio>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket_utils.hpp"

#define DELIM_CHAR (char)0

#define PORT 8080

int main(int argc, char const *argv[])
{
	simple_string::string_buffer s, buff;
	int sock;
	sockaddr_in serv_addr, clie_addr;

	serv_addr.sin_family				= AF_INET;
	serv_addr.sin_port					= htons(8080);
	inet_pton( AF_INET, "127.0.0.1", &serv_addr.sin_addr );

	sock = socket( AF_INET, SOCK_STREAM, 0 );
	// bind( sock, (sockaddr *)&clie_addr, sizeof( clie_addr ) );

	while( !connect( sock, (sockaddr *)&serv_addr, sizeof( serv_addr ) ) ) continue;

	while(1)
	{
		printf(">> ");
		s.reset(); s.input('\n');
		s[s.len - 1] = DELIM_CHAR;

		send( sock, s.get(), s.len, 0 );
		if(s[0] == 'q' && s.len == 2) break;

		buff.reset(); buff.recv_from( sock, DELIM_CHAR );
		printf("%d\t%s\n", (int)buff.len, buff.get());
	}

	printf("CLOSING CONNECTION\n");
	close( sock );

	return 0;
}
