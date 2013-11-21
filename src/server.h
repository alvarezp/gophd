#ifndef gophd_server_h
#define gophd_server_h

int start_server(unsigned int port, unsigned int queue_size );
int end_server(int fd);
int close_socket(int fd, int linger);
int get_line( char * buf, size_t size );
    
#endif
