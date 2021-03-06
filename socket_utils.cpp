#include <cstring>
#include <arpa/inet.h>
#include "socket_utils.hpp"

#define MSG_BLOCK_LEN 64

namespace simple_string {
string_buffer::string_buffer ()
{
    ptr = new char[1];
    len = 0, cap = 1;
    ptr[0] = 0;
}

void
string_buffer::append (const char & other)
{
    len++;
    if (len >= cap) {
        cap <<= 1;
        char * new_ptr = new char[cap];

        memcpy (new_ptr, ptr, len);

        delete[] ptr;
        ptr = new_ptr;
    }
    ptr[len - 1] = other;
    ptr[len]     = 0;
}

void
string_buffer::append (const char * other, size_t sz)
{
    len += sz;
    if (len >= cap) {
        while (len >= cap)
            cap <<= 1;
        char * new_ptr = new char[cap];

        memcpy (new_ptr, ptr, len);

        delete[] ptr;
        ptr = new_ptr;
    }

    memcpy (ptr + len - sz, other, sz);
    ptr[len] = 0;
}

void
string_buffer::reset ()
{
    len    = 0;
    ptr[0] = 0;
}

char *&
string_buffer::get ()
{
    return ptr;
}

char &
string_buffer::operator[] (const size_t & index)
{
    return ptr[index];
}

string_buffer::~string_buffer ()
{
    delete[] ptr;
}
}                    // namespace simple_string

namespace simple_callback {
callback::callback (int s, int (*f) (callback *, const int &, const sockaddr_in &, const int &))
{
    sock = s;
    func = f;
}
}                    // namespace simple_callback

namespace converters {
u_int32_t
host_to_network_addr (u_int32_t a, u_int32_t b, u_int32_t c, u_int32_t d)
{
    u_int32_t res = 0;
    res           = a | (b << 8) | (c << 16) | (d << 24);
    return res;
}

char *
network_to_host_addr (u_int32_t addr)
{
    u_int32_t a   = addr & 255;
    u_int32_t b   = (addr >> 8) & 255;
    u_int32_t c   = (addr >> 16) & 255;
    u_int32_t d   = (addr >> 24) & 255;

    char *    res = new char[16];
    res[15]       = 0;
    res[3] = res[7] = res[11] = '.';

    res[0]                    = '0' + (char) (a / 100);
    res[1]                    = '0' + (char) ((a / 10) % 10);
    res[2]                    = '0' + (char) (a % 10);

    res[4]                    = '0' + (char) (b / 100);
    res[5]                    = '0' + (char) ((b / 10) % 10);
    res[6]                    = '0' + (char) (b % 10);

    res[8]                    = '0' + (char) (c / 100);
    res[9]                    = '0' + (char) ((c / 10) % 10);
    res[10]                   = '0' + (char) (c % 10);

    res[12]                   = '0' + (char) (d / 100);
    res[13]                   = '0' + (char) ((d / 10) % 10);
    res[14]                   = '0' + (char) (d % 10);

    return res;
}

u_int16_t
host_to_network_port (u_int16_t p)
{
    u_int16_t right = (p >> 8) & 255;
    return (p << 8) | right;
}

u_int16_t
network_to_host_port (u_int16_t p)
{
    u_int16_t left = p & 255;
    return (p >> 8) | (left << 8);
}
}                    // namespace converters
