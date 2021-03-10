#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

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
		printf(">> ");
		s.reset(); s.input('\n');
		s[s.len - 1] = ';';

		send( sock, s.get(), s.len, 0 );
		if(s[0] == 'q' && s.len == 2) break;

		buff.reset(); buff.recv_from( sock, ';' );
		printf("%d\t%s\n", (int)buff.len, buff.get());
	}

	printf("CLOSING CONNECTION\n");
	close( sock );

	return 0;
}
