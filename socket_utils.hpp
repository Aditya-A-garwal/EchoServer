namespace simple_string
{
    struct string_buffer
    {
        char *ptr;
        size_t len , cap;

        string_buffer( );
        void append( const char & );
        void append( const char * , size_t );
        void reset( );
        void input( const char );
        int recv_from( const int , const char );
        int recv_from_include( const int , const char );
        char *&get();
        char &operator[]( const size_t & );
        ~string_buffer( );
    };
}

namespace simple_callback
{
    struct callback
    {
        int sock;
        int (*func)(const int &, const int &, const sockaddr_in &, const int &);

        callback(int, int (*)(const int &, const int &, const sockaddr_in &, const int &));
    };
}

namespace converters
{
    u_int32_t host_to_network_addr( u_int32_t , u_int32_t , u_int32_t , u_int32_t );

    char *network_to_host_addr( u_int32_t );

    u_int16_t host_to_network_port( u_int16_t );

    u_int16_t network_to_host_port( u_int16_t );
}