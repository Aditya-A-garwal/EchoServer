#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <sys/epoll.h>

#include <arpa/inet.h>

#include <unistd.h>

#define PORT 8080
#define MAX_EVENTS 65535

struct effic_string
{
	char 	*ptr;

	size_t	len , size , cap;

	effic_string()
	{
		ptr = new char[2];
		len = 0, size = 1, cap = 2;
		ptr[0] = ptr[1] = 0;
	}

	void operator+=(const char &other)
	{
		this->append(other);
	}

	void append(const char &other)
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

	void input(const char delim)
	{
		char c = delim + 1;
		while(c != delim)
		{
			c = getchar();
			this->append(c);
		}
	}

	int recv_from(const int sd, const char delim)
	{
		char c = delim + 1;
		int num = 0;

		while(1)
		{
			num += recv( sd, &c, 1, 0 );
			if(c == delim) return (num - 1);
			this->append(c);
		}

		return num;
	}

	int recv_from_include(const int sd, const char delim)
	{
		char c = delim + 1;
		int num = 0;

		while(c != delim)
		{
			num += recv( sd, &c, 1, 0);
			this->append(c);
		}

		return num;
	}

	char *&get()
	{  return ptr;  }

	char &operator[](const size_t &index)
	{  return ptr[index];  }
};

u_int32_t host_to_network_addr(u_int32_t a, u_int32_t b, u_int32_t c, u_int32_t d)
{
	u_int32_t res = 0;
	res = a | (b << 8) | (c << 16) | (d << 24);
	return res;
}

char *network_to_host_addr(u_int32_t addr)
{
	u_int32_t a = addr & 255;
	u_int32_t b = (addr >> 8) & 255;
	u_int32_t c = (addr >> 16) & 255;
	u_int32_t d = (addr >> 24) & 255;

	char* res = new char[16];
	res[15] = 0;
	res[3] = res[7] = res[11] = '.';

	res[0] = '0' + (char)(a/100);
	res[1] = '0' + (char)((a/10) % 10);
	res[2] = '0' + (char)(a % 10);

	res[4] = '0' + (char)(b/100);
	res[5] = '0' + (char)((b/10) % 10);
	res[6] = '0' + (char)(b % 10);

	res[8] = '0' + (char)(c/100);
	res[9] = '0' + (char)((c/10) % 10);
	res[10] = '0' + (char)(c % 10);

	res[12] = '0' + (char)(d/100);
	res[13] = '0' + (char)((d/10) % 10);
	res[14] = '0' + (char)(d % 10);

	return res;
}

u_int16_t host_to_network_port(u_int16_t p)
{
	u_int16_t right = (p >> 8) & 255;
	return (p << 8) | right;
}

u_int16_t network_to_host_port(u_int16_t p)
{
	u_int16_t left = p & 255;
	return (p >> 8) | (left << 8);
}

int main(int argc , char *argv[])
{
	int opt = 1, listener , addrlen , new_socket , num_events, i , num_recv , sd;
	sockaddr_in address;
	effic_string buffer;
	epoll_event primary_event, events[MAX_EVENTS], *secondary_event;
	char c;

	// create a socket and set its options
	listener = socket(AF_INET , SOCK_STREAM , 0);
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, 4);

	// set the type of the socket
	address.sin_family = AF_INET;
	*(u_int32_t *)&(address.sin_addr) = host_to_network_addr(127, 0, 0, 1);
	address.sin_port = host_to_network_port( PORT );
	addrlen = sizeof(address);
	// address.sin_addr.s_addr = INADDR_ANY;

	// bind the socket to a fixed port and start listening
	bind(listener, (struct sockaddr *)&address, sizeof(address));
	listen(listener, 65535);
	printf("Listener on port %d \nWaiting for connections\n", PORT);

	primary_event.data.fd = listener;
	primary_event.events = EPOLLIN;

	int epoll_fd = epoll_create1(0);
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener, &primary_event);

	while(1)
	{
		num_events = epoll_wait( epoll_fd, events, MAX_EVENTS, 0 ); // get the number of events

		for(int i = 0; i < num_events; i++)
		{
			sd = events[i].data.fd;
			if(sd == listener)
			{
				new_socket = accept(listener, (struct sockaddr *)&address, (socklen_t*)&addrlen);
				printf("New connection, socket : %d, ip : %s, port : %d\n" , new_socket , network_to_host_addr(*(int *)&(address.sin_addr)) , network_to_host_port(address.sin_port));

				secondary_event = new epoll_event[1];
				secondary_event->data.fd = new_socket;
				secondary_event->events = EPOLLIN;

				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, secondary_event);
			}
			else
			{
				buffer.reset(); buffer.recv_from_include( sd, ';' );

				if(buffer.len == 2 && buffer[0] == 'q')
				{
					getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
					printf("Host disconnected, ip %s, port %d \n", network_to_host_addr(*(int *)&(address.sin_addr)) , network_to_host_port(address.sin_port));

					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sd, NULL);
					close( sd );
				}
				else
				{
					printf("Sending to %d: %s\n", sd, buffer.get());
					send( sd , buffer.get() , buffer.len , 0 );
				}
			}
		}
	}

	close(epoll_fd);
	return 0;
}
