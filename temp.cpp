#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <sys/socket.h>
#include <sys/epoll.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_EVENTS 65535

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

int main(int argc , char *argv[])
{
	int opt = 1, listener , addrlen , new_socket , activity, i , num_recv , sd;
	sockaddr_in address;
	effic_string buffer;
	epoll_event primary_event, events[MAX_EVENTS], *secondary_event;

	listener = socket(AF_INET , SOCK_STREAM , 0);

	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, 4);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	bind(listener, (struct sockaddr *)&address, sizeof(address));
	printf("Listener on port %d \n", PORT);
	listen(listener, 65535);

	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	primary_event.data.fd = listener;
	primary_event.events = EPOLLIN;

	int epoll_fd = epoll_create1(0);
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener, &primary_event);

	while(1)
	{
		activity = epoll_wait( epoll_fd, events, MAX_EVENTS, 0);

		for(int i = 0; i < activity; i++)
		{
			sd = events[i].data.fd;
			if(sd == listener)
			{
				new_socket = accept(listener, (struct sockaddr *)&address, (socklen_t*)&addrlen);
				printf("New connection, socket : %d, ip : %s, port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

				secondary_event = new epoll_event[1];
				secondary_event->data.fd = new_socket;
				secondary_event->events = EPOLLIN;

				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, secondary_event);
			}
			else
			{
				char c;
				num_recv = 0;
				buffer.reset();

				while(1)
				{
					num_recv += recv( sd, &c, 1, 0 );
					buffer += c;
					if(c == ';') break;
				}
				if(num_recv == 2 && buffer[0] == 'q')
				{
					getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sd, events + i);
					close( sd );
				}
				else
				{
					printf("Sending to %d: %s\n", sd, buffer.get());
					send(sd , buffer.get() , buffer.len , 0 );
				}
			}
		}
	}

	close(epoll_fd);
	return 0;
}
