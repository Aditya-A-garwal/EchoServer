#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/epoll.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080

struct effic_string
{
	char 	*ptr;

	size_t	len,  size,  cap;

	effic_string()
	{
		ptr = new char[2];
		len = 0, size = 1, cap = 2;
		ptr[0] = ptr[1] = 0;
	}

	void operator+=(const char &other)
	{
		len ++, size ++;
		if(size >= cap)
		{
			cap <<= 1;
			char *newPtr = new char[cap];
			memset(newPtr, 0, cap);
			memcpy(newPtr, ptr, size);
			delete[] ptr;
			ptr = newPtr;
		}
		ptr[len - 1] = other, ptr[size - 1] = 0;
	}

	void reset()
	{
		len = 0, size = 1;
		ptr[0] = ptr[1] = 0;
	}

	char *&get()
	{  return ptr;  }

	char &operator[](const size_t &index)
	{  return ptr[index];  }
};

int max(int a, int b)
{
	return (a > b)?(a):(b);
}

int main(int argc , char *argv[])
{
	int opt = 1;
	int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd , max_sd;
	sockaddr_in address;
	fd_set readfds;
	effic_string buffer;

	memset(client_socket, -1, sizeof(client_socket));
	master_socket = socket(AF_INET , SOCK_STREAM , 0);

	setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, 4);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	// *(u_int32_t *)&(address.sin_addr.s_addr) = ;
	address.sin_port = htons( PORT );

	bind(master_socket, (struct sockaddr *)&address, sizeof(address));
	printf("Listener on port %d \n", PORT);
	listen(master_socket, 65535);

	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	FD_ZERO(&readfds);
	FD_SET(master_socket, &readfds);

	while(1)
	{
		max_sd = master_socket;
		for ( i = 0 ; i < max_clients ; i++) max_sd = max(max_sd, client_socket[i]);
		activity = select( ++max_sd , &readfds , NULL , NULL , NULL );

		if(FD_ISSET( master_socket, &readfds ))
		{
			new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
			printf("New connection, socket: %d , ip : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
			for(int i = 0; i < max_clients; i++)
			{
				if(client_socket[i] == -1)
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
					FD_SET( new_socket, &readfds );
					break;
				}
			}
		}

		for(int i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];

			if(FD_ISSET( sd, &readfds ))
			{
				char c;
				valread = 0;
				buffer.reset();
				while(1)
				{
					valread += recv( sd, &c, 1, 0 );
					buffer += c;
					if(c == ';') break;
				}
				if(valread == 2 && buffer[0] == 'q')
				{
					getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

					FD_CLR( sd, &readfds );
					close( sd );
					client_socket[i] = -1;
				}
				else
				{
					printf("Sending: %s\n", buffer.get());
					send(sd , buffer.get() , buffer.len , 0 );
				}
			}
		}
	}
	return 0;
}
