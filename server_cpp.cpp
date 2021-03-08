#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

u_int32_t host_to_network_addr(u_int32_t a, u_int32_t b, u_int32_t c, u_int32_t d)
{
	u_int32_t res = 0;
	res = a | (b << 8) | (c << 16) | (d << 24);
	return res;
}

u_int16_t host_to_network_port(u_int16_t p)
{
	u_int16_t right = (p >> 8) & 255;
	return (p << 8) | right;
}

int main(int argc, char const *argv[])
{
	int serv, conn, opt = 1;
	int num_recv;
	effic_string s;
	char c;

	sockaddr_in address;
	int addrlen = sizeof(address);

    serv = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serv, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family					= AF_INET;
    *(u_int32_t *)&(address.sin_addr)	= host_to_network_addr(127, 0, 0, 1);
    address.sin_port					= host_to_network_port(PORT);

    bind(serv, (sockaddr *)&address, sizeof(address));
    listen(serv, 0);
	conn = accept(serv, (struct sockaddr *)&address, (socklen_t*)&addrlen);

	while(1)
	{
		num_recv = 0;
		s.reset();
		while(1)
		{
			num_recv += recv( conn, &c, 1, 0 );
			s += c;
			if(c == ';') break;
		}

		printf("%s\n", s.get());
		if(s.len == 2 && s[0] == 'q') break;

		send( conn , s.get() , s.len , 0 );
	}

	printf("CLOSING CONNECTION\n");

    return 0;
}
