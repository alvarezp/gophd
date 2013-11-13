#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>

#include "utils.h"
#include "types.h"
#include "server.h"

#define CONNECTION_QUEUE 1024
#define LISTEN_PORT 70
#define BUFFER_SIZE 1024
#define LF 10
#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT 70
#define GOPHER_ROOT "/Users/louis77/"

char * resolve_selector( char * filepath, const char * selector );
int is_menu(const char * selector);
int is_file(const char * selector);
int is_dir(const char * selector);
void print_menu_item(int fd, menu_item * item);
void print_directory(int fd, const char * dirname);
void print_textfile(int fd, const char * filename);
void print_message(int fd, const char * message);
void print_closing(int fd);
void print_nl(int fd);
void main_shutdown();

int main(int argc, const char * argv[])
{
    plog("Starting gophd...");
    int fd = 0;
    if( (fd = start_server( LISTEN_PORT, CONNECTION_QUEUE )) < 0 ){
        perror(NULL);
        abort();
    }
    
    // Accept connections, blocking
    int accept_fd = 0;
    struct sockaddr *addr = NULL;
    socklen_t *length_ptr = NULL;
    
    char * buf = malloc(BUFFER_SIZE);
    atexit(&main_shutdown);
    
    while( (accept_fd = accept(fd, addr, length_ptr)) != -1 ) {
        // Need to read the address here
        // we must switch this to non-blocking read
        int run = 1;
        int ptr = 0;
        ssize_t got_bytes = 0;
    
        while( run ) {
            got_bytes = read(accept_fd, buf + ptr, BUFFER_SIZE - ptr);
            if(got_bytes <= 0){
                buf[ptr] = 0; // Terminate string
                break;
            }
            ptr += got_bytes;
            if( get_line(buf, ptr) ){
                break;
            }
        }
        
        char * selector = resolve_selector(NULL, buf);
        plog("Request: %s, resolved: %s", buf, selector);
        
        // this is for development only
        if(strcmp(buf, "/exit") == 0)
            break;

        if( is_menu(selector) ) {
            plog("Sending menu");
            print_message(accept_fd, "Welcome to Gophd!");
            print_directory(accept_fd, GOPHER_ROOT);
        } else if ( is_file(selector) ) {
            plog("Sending file");
            print_textfile(accept_fd, selector);
        } else if ( is_dir(selector) ) {
            plog("Sending dir");
            print_directory(accept_fd, selector);
        }

        free(selector);
        close_socket(accept_fd, 1);
        
    }
    free(buf);
    
    // End server
    if( end_server(fd) < 0 ) {
        perror(NULL);
        return EXIT_FAILURE;
    };
    plog("Socket released");
    return EXIT_SUCCESS;
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
    size_t len = strlen(GOPHER_ROOT) + strlen(selector) + 3;  // two more bytes for slashes and \0
    if( filepath == NULL ) filepath = malloc(len);
    
    unsigned int start_at = 0;
    if( selector[0] == '/' ) start_at = 1;
    asprintf(&filepath, "%s%s", GOPHER_ROOT, selector+start_at);
    return filepath;
}


int is_menu(const char * selector){
    if( strcmp(selector, GOPHER_ROOT) == 0 ) return 1;
    return 0;
}

int is_file(const char * selector){
    struct stat * fstat = malloc(sizeof(struct stat));
    int success = 0;
    success = stat(selector, fstat) == 0 && S_ISREG(fstat->st_mode);
    free(fstat);
    return success;
}

int is_dir(const char * selector){
    struct stat * fstat = malloc(sizeof(struct stat));
    int success = 0;
    success = stat(selector, fstat) == 0 && S_ISDIR(fstat->st_mode);
    free(fstat);
    return success;
}

/** Send stub menu
 */

void print_menu_item(int fd, menu_item * item){
    dprintf(fd, "%c%s\t%s\t%s\t%d", item->type, item->display, item->selector, item->host, item->port);
    print_nl(fd);
}


/** Read directory items
 *  and return array of dirents
 * 
 */

void print_directory(int fd, const char * dirname){
    DIR * dir = opendir(dirname);
    struct dirent * entry = NULL;
    
    if (dir == NULL){
        perror(NULL);
        return;
    }
    
    while( (entry = readdir(dir)) != NULL ){
        if( (*entry).d_name[0] == '.' ) continue; // don't print hidden files/dirs
            
        char type;
        switch( entry->d_type ){
            case DT_REG: type = ITEM_FILE; break;
            case DT_DIR: type = ITEM_DIR; break;
            default: continue; // don't send item if not either file or dir
        }
        
        char *selector;
        asprintf(&selector, "/%s", entry->d_name);
        
        menu_item * item = menu_item_new(type, entry->d_name, selector, DEFAULT_HOST, DEFAULT_PORT);
        print_menu_item(fd, item);
        menu_item_free(item);
        
    }
    
    print_closing(fd);
    if( closedir(dir) != 0 )
        perror(NULL);
}

/** Read a file and print it line by line to the server
 */

void print_textfile(int fd, const char * filename){
    FILE * file = fopen(filename, "r");
    if (file == NULL) return;
    
    size_t bufsize = BUFFER_SIZE;
    char *lineptr = malloc(bufsize);
    ssize_t retsize = 0;
    while ((retsize = getline(&lineptr, &bufsize, file)) != -1) {
        write(fd, lineptr, retsize);
    }
    free(lineptr);
    fclose(file);
}


/** Print informational message
  */

void print_message(int fd, const char * message){
    dprintf(fd, "i%s", message);
    print_nl(fd);
}


void print_nl(int fd){
    write(fd, "\r\n", 2);
}

void print_closing(int fd){
    write(fd, ".", 1);
    print_nl(fd);
}

/** Terminate the program by releasing all resources
 */

void main_shutdown(){
    plog("Program ended.");
}