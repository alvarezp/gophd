#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>

#include "utils.h"
#include "types.h"
#include "server.h"
#include "gophermap.h"

#define CONNECTION_QUEUE 1024
#define LISTEN_PORT 70
#define BUFFER_SIZE 1024
#define LF 10
#define CRLF "\r\n"
#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT 70
#define GOPHER_ROOT "/Users/louis/"
#define GOPHERMAP_FILENAME "Gophermap"

char * resolve_selector( char * filepath, const char * selector );
enum item_types resolve_item(struct dirent * entry);
int is_menu(struct request_t * req);
void print_menu_item(int fd, menu_item * item);
void print_directory(struct request_t * req);
void print_file(struct request_t * req);
void print_message(int fd, const char * message);
void print_closing(int fd);
void main_shutdown();
void * handle_request(void * args);

int main(int argc, const char * argv[])
{
    plog("Starting gophd...");
    int fd;
    if( (fd = start_server( LISTEN_PORT, CONNECTION_QUEUE )) < 0 ){
        perror(NULL);
        abort();
    }
    
    // Accept connections, blocking
    int accept_fd;
    struct sockaddr *addr = NULL;
    socklen_t *length_ptr = NULL;
    
    atexit(&main_shutdown);
    
    while( (accept_fd = accept(fd, addr, length_ptr)) != -1 ) {
        int * req_fd = malloc(sizeof(int));
        *req_fd = accept_fd;
        pthread_t thread;
        if( pthread_create(&thread, NULL, handle_request, req_fd) != 0 ){
            plog("Could not create thread, continue non-threaded...");
            handle_request(req_fd);
        }
        free(addr);
        free(length_ptr);
    }
    
    // End server
    if( end_server(fd) < 0 ) {
        perror(NULL);
        return EXIT_FAILURE;
    };
    plog("Socket released");
    return EXIT_SUCCESS;
}


/** Handle a request, thread-safe
  */

void * handle_request(void * args){
    int * fd = (int *)args;
    int run = 1;
    int ptr = 0;
    ssize_t got_bytes = 0;
    char * buf = malloc(BUFFER_SIZE);
    while( run ) {
        got_bytes = read( *fd, buf + ptr, BUFFER_SIZE - ptr);
        if(got_bytes <= 0){
            buf[ptr] = 0; // Terminate string
            break;
        }
        ptr += got_bytes;
        if( get_line(buf, ptr) ){
            break;
        }
    }
    
    struct request_t * req = malloc(sizeof(struct request_t));
    req->fd = *fd;
    req->selector = buf;
    req->selector_len = strlen(req->selector);
    req->path = resolve_selector(NULL, req->selector);
    req->path_len = strlen(req->path);
    
    plog("Request: %s", buf);
    
    if( is_menu( req ) ) {
        char * gopherfile;
        asprintf(&gopherfile, "%s%s", req->path, GOPHERMAP_FILENAME);
        plog("Sending menu @ %s", gopherfile);
        menu_item * items[1024];
        int item_count = parse_gophermap( gopherfile, &items[0], DEFAULT_HOST, DEFAULT_PORT );
        plog("Gophermap parsed, items: %u", item_count);
        for( int i = 0; i < item_count; i++ ){
            print_menu_item( req->fd, items[i] );
        }
        free( gopherfile) ;
    } else if ( is_file( req->path ) ) {
        plog("Sending file");
        print_file(req);
    } else if ( is_dir( req->path ) ) {
        plog("Sending dir");
        print_directory(req);
    }
    
    close_socket(req->fd, 1);
    free(req->selector);
    free(req->path);
    free(args);
    pthread_exit(NULL);
}

/** Resolve selector string to Filesystem path
 *  Uses GOPHER_ROOT as base directory
 *
 *  Input parameter:    char * filepath , IFNULL will be alloc'd
 *                      const char * selector
 *  Return value:       char * filepath
 * 
 *  You must free() filepath after usage
 */

char * resolve_selector( char * filepath, const char * selector ){
    asprintf(&filepath, "%s%s", GOPHER_ROOT, selector + (selector[0] == '/' ? 1 : 0));
    return filepath;
}

int is_menu(struct request_t * req){
    int success = 0;

    if ( is_dir(req->path) ){
        char * gopherfile = malloc( sizeof(char)*(req->path_len+strlen(GOPHERMAP_FILENAME)+2) );

        strcpy(gopherfile, req->path);
        if ( req->path[req->path_len-1] != '/' )
            strcat(gopherfile, "/");
        strcat(gopherfile, GOPHERMAP_FILENAME);

        success = exists(gopherfile);
        free(gopherfile);
    }
    return success;
}

enum item_types resolve_item(struct dirent * entry){
    enum item_types type;
    switch( entry->d_type ){
        case DT_REG:
            if( str_ends_with(entry->d_name, ".zip") ) {
                type = ITEM_ARCHIVE;
            } else if ( str_ends_with(entry->d_name, ".jpg" )) {
                type = ITEM_IMAGE;
            } else if ( str_ends_with(entry->d_name, ".png" )) {
                type = ITEM_PNG;
            } else if ( str_ends_with(entry->d_name, ".pdf" )) {
                type = ITEM_PDF;
            } else if ( str_ends_with(entry->d_name, ".gif" )) {
                type = ITEM_GIF;
            } else {
                type = ITEM_FILE; // text file?
            }
            break;
        case DT_DIR: type = ITEM_DIR; break;
        default: type = NO_ITEM;
    }
    return type;
}

/** Print functions
 */

void print_directory(struct request_t * req){
    DIR * dir = opendir(req->path);
    struct dirent * entry = NULL;
    
    if (dir == NULL){
        perror(NULL);
        return;
    }
    
    while( (entry = readdir(dir)) != NULL ){
        if( (*entry).d_name[0] == '.' ) continue; // don't print hidden files/dirs
            
        char type = resolve_item(entry);
        char * sel = NULL;
        asprintf(&sel, "%s/%s", req->selector, entry->d_name);
        
        menu_item * item = menu_item_new(type, entry->d_name, sel, DEFAULT_HOST, DEFAULT_PORT);
        print_menu_item(req->fd, item);
        menu_item_free(item);
        free(sel);
    }
    
    print_closing(req->fd);
    if( closedir(dir) != 0 )
        perror(NULL);
}

void print_file(struct request_t * req){
    FILE * file = fopen(req->path, "rb");
    
    if (file != NULL) {
        fseek(file, 0L, SEEK_END);     /* Position to end of file */
        long file_len = ftell(file);        /* Get file length */
        rewind(file);
        
        char * lineptr = malloc(file_len);
        if (fread(lineptr, file_len, 1, file) == 1) {
            send(req->fd, lineptr, file_len, 0);
        }

        perror(NULL);
        free(lineptr);
        fclose(file);
    }
}

void print_menu_item(int fd, menu_item * item){
    dprintf(fd, "%c%s\t%s\t%s\t%d%s", item->type, item->display, item->selector, item->host, item->port, CRLF);
}

void print_message(int fd, const char * message){
    dprintf(fd, "i%s%s", message, CRLF);
}

void print_closing(int fd){
    dprintf(fd, ".%s", CRLF);
}

/** Terminate the program by releasing all resources
 */

void main_shutdown(){
    pthread_exit(NULL);
    plog("Program ended.");
}
