#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "server.h"

void catch_sigpipe(int sig){
    printf("Broken pipe, continue...");
}

/** Creates a listening socket on port PORT and returns the file descriptor.
 *
 *  Input parameters:
 *  port    The port to listen on
 *
 *  Return value: the file descriptor of the listening socket
 *
 *  Call end_server() clean up before you exit.
 */

int start_server(unsigned int port, unsigned int queue_size ){
    int fd = 0;
    
    signal(SIGPIPE, catch_sigpipe);
    
    // Create a socket
    fd = socket( PF_INET, SOCK_STREAM, 0 );
    if( fd == -1 ) return fd;
    
    printf("File descriptor for socket is %d\n", fd);
    int opts = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));
    
    // Bind the socket
    // TODO This will prevent us to start up multiple servers
    static struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons( port );
    serv_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    if (bind(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        return -1;
    printf("Socket bound to port %d\n", port);
    
    // Make it a listening socket
    if( listen(fd, queue_size) == -1 )
        return -1;
    return fd;
}


/** Ends listening server and frees resources.
 *
 *  Return value: 0 = success, -1 = failure
 */

int end_server(int fd){
    shutdown(fd, 2);
    return close( fd );
}


/** Close a socket and set it optionally to linger.
 *
 *  Input Parameter:
 *  int linger - 0 false, 1 true
 *
 *  Return Parameter: 0 - failure, 1 - success
 */

int close_socket(int fd, int linger){
    if (linger){
        struct linger opts;
        opts.l_onoff = 1;
        opts.l_linger = 30;
        if( setsockopt(fd, SOL_SOCKET, SO_LINGER, &opts, sizeof(opts)) < 0 ){
            perror(NULL);
        }
    }
    
    if( close( fd ) < 0 ) {
        perror(NULL);
        return 0;
    }
    return 1;
}

/** Search for a line delimited by CRLF, and replace CRLF by a null terminator.
 *
 * Input parameter:
 * char * buf - buffer to search for a full line, returns buffer without line
 * size_t * size - size of buffer
 *
 * Return value: 1 for success, 0 for failure
 *
 * Don't forget to free() returned buffer
 */

int get_line( char * buf, size_t size ){
    char * pos = memchr(buf, '\n', size);
    if( pos != NULL ){
        *(pos-1) = 0;
        return 1;
    }
    return 0;
}