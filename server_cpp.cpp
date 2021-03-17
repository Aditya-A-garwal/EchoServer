#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_EVENTS 65535

struct string_buffer
{
	char 	*ptr;

	size_t	len , size , cap;

	string_buffer()
	{
		ptr = new char[2];
		len = 0, size = 1, cap = 2;
		ptr[0] = ptr[1] = 0;
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
		int num_recv = 0;

		while(1)
		{
			num_recv += recv( sd, &c, 1, 0 );
			if(c == delim) return (num_recv - 1);
			this->append(c);
		}

		return num_recv;
	}

	int recv_from_include(const int sd, const char delim)
	{
		char c = delim + 1;
		int num_recv = 0;

		while(c != delim)
		{
			num_recv += recv( sd, &c, 1, 0);
			this->append(c);
		}

		return num_recv;
	}

	char *&get()
	{  return ptr;  }

	char &operator[](const size_t &index)
	{  return ptr[index];  }
};

struct callback
{
	int sock;
	int (*func)(const int &, const int &, const sockaddr_in &, const int &);

	callback(int s, int (*f)(const int &, const int &, const sockaddr_in &, const int &))
	{
		sock = s;
		func = f;
	}
};

string_buffer buffer;

int __echo(const int &sd, const int &epoll_fd, const sockaddr_in &address, const int &addrlen)
{
	buffer.reset(); buffer.recv_from_include( sd, ';' );

	if(buffer.len == 2 && buffer[0] == 'q')
	{
		getpeername(sd , (struct sockaddr*)&address, (socklen_t *)&addrlen);
		printf("Host disconnected, ip %s, port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sd, NULL);
		close( sd );

		return 0;
	}
	else
	{
		printf("Sending to %d: %s\n", sd, buffer.get());
		send( sd , buffer.get() , buffer.len , 0 );
	}

	return 1;

}

int __accept(const int &listener, const int &epoll_fd, const sockaddr_in &address, const int &addrlen)
{
	int new_socket = accept(listener, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	printf("New connection, socket %d, ip %s, port %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

	epoll_event secondary_event;
	secondary_event.data.ptr = (void *)(new callback(new_socket, &__echo));
	secondary_event.events = EPOLLIN;

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &secondary_event);

	return 1;
}

int main(int argc , char *argv[])
{
	int32_t listener , new_socket , sd , epoll_fd;
	sockaddr_in address;
	u_int32_t addrlen;
	epoll_event primary_event, events[MAX_EVENTS];

	// create a socket and set its options
	listener = socket(AF_INET , SOCK_STREAM , 0);

	// int opt = 1;
	// setsockopt(listener, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, 4);

	address.sin_family = AF_INET;
	address.sin_port = htons( PORT );
	address.sin_addr.s_addr = INADDR_ANY;
	// inet_pton(AF_INET, "192.168.1.22", &address.sin_addr);
	addrlen = sizeof(address);

	bind(listener, (sockaddr *)&address, sizeof(address));
	listen(listener, 65535);
	printf("Listener on port %d \nWaiting for connections\n", PORT);

	epoll_fd = epoll_create1(0);
	callback cb1(listener, &__accept);
	primary_event.data.ptr = (void *)&cb1;
	primary_event.events = EPOLLIN;

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener, &primary_event);

	while(1)
	{
		int32_t num_events = epoll_wait( epoll_fd, events, MAX_EVENTS, -1 );
		for(int i = 0; i < num_events; i++)
		{
			callback *cb = (callback *)(events[i].data.ptr);
			int state = cb->func(cb->sock, epoll_fd, address, addrlen);
			if(!state) delete cb;
		}
	}

	close(epoll_fd);
	close(listener);

	return 0;
}
