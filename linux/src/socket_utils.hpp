namespace simple_string {
struct string_buffer {
    char * ptr;
    size_t len, cap;

    string_buffer ();

    void    append (const char &);

    void    append (const char *, size_t);

    void    reset ();

    char *& get ();

    char &  operator[] (const size_t &);

    ~string_buffer ();
};
}                    // namespace simple_string

namespace simple_callback {
struct callback {
    int sock;
    int (*func) (simple_callback::callback *, const int &, const sockaddr_in &, const int &);
    simple_string::string_buffer s;

    callback (int, int (*) (simple_callback::callback *, const int &, const sockaddr_in &, const int &));
};
}                    // namespace simple_callback

namespace converters {
u_int32_t host_to_network_addr (u_int32_t, u_int32_t, u_int32_t, u_int32_t);

char *    network_to_host_addr (u_int32_t);

u_int16_t host_to_network_port (u_int16_t);

u_int16_t network_to_host_port (u_int16_t);
}                    // namespace converters