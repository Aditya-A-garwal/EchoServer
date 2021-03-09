#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

struct effic_string
{
	char 	*ptr;

	size_t	len;
	size_t	size;
	size_t	cap;

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

u_int32_t network_to_host_addr(u_int32_t addr)
{
	return addr;
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

effic_string s, buff;
int32_t sock, num_recv;

sockaddr_in serv_addr, clie_addr;

int main(int argc, char const *argv[])
{
	serv_addr.sin_family				= AF_INET;
	serv_addr.sin_port					= host_to_network_port(8080);
	*(u_int32_t*)&(serv_addr.sin_addr)	= host_to_network_addr(127, 0, 0, 1);

	while( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0) continue;
	// bind( sock, (sockaddr *)&clie_addr, sizeof( clie_addr ) );

	while( !connect( sock, (sockaddr *)&serv_addr, sizeof( serv_addr ) ) ) continue;

	while(1)
	{
		char c;
		s.reset(); buff.reset();
		num_recv = 0;

		printf(">> ");
		while(1)
		{
			c = getchar();
			if(c == '\n') { s += ';'; break; }
			s += c;
		}

		send( sock, s.get(), s.len, 0 );
		if(s[0] == 'q' && s.len == 2) break;

		while(1)
		{
			num_recv += recv( sock, &c, 1, 0 );
			if(c == ';') { num_recv -= 1; break; }
			buff += c;
		}

		printf("%d\t%s\n", num_recv, buff.get());
	}

	printf("CLOSING CONNECTION\n");
	shutdown( sock, SHUT_RDWR );

	return 0;
}
