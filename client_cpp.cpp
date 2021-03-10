#include <cstring>
#include <cstdio>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

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

sockaddr_in serv_addr, clie_addr;

int main(int argc, char const *argv[])
{

	string_buffer s, buff;
	int32_t sock;

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
