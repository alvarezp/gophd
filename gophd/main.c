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
#define CRLF "\r\n"
#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT 70
#define GOPHER_ROOT "/Users/louis77/"

char * resolve_selector( char * filepath, const char * selector );
enum item_types resolve_item(struct dirent * entry);
int is_menu(const char * selector);
int is_file(const char * selector);
int is_dir(const char * selector);
void print_menu_item(int fd, menu_item * item);
void print_directory(int fd, const char * dirname);
void print_file(int fd, const char * filename);
void print_message(int fd, const char * message);
void print_closing(int fd);
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

        plog("Request: %s", buf);
        
        // this is for development only
        if(strcmp(buf, "/exit") == 0)
            break;

        if( is_menu(buf) ) {
            plog("Sending menu");
            print_message(accept_fd, "Welcome to Gophd!");
            print_directory(accept_fd, buf);
        } else if ( is_file(buf) ) {
            plog("Sending file");
            print_file(accept_fd, buf);
        } else if ( is_dir(buf) ) {
            plog("Sending dir");
            print_directory(accept_fd, buf);
        }

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
    if( strlen(selector) == 0 ) return 1;
    return 0;
}

int is_file(const char * selector){
    char * path = resolve_selector(NULL, selector);
    struct stat * fstat = malloc(sizeof(struct stat));
    int success = stat(path, fstat) == 0 && S_ISREG(fstat->st_mode);
    free(fstat);
    free(path);
    return success;
}

int is_dir(const char * selector){
    char * path = resolve_selector(NULL, selector);
    struct stat * fstat = malloc(sizeof(struct stat));
    int success = 0;
    success = stat(path, fstat) == 0 && S_ISDIR(fstat->st_mode);
    free(fstat);
    free(path);
    return success;
}

/** Send stub menu
 */

void print_menu_item(int fd, menu_item * item){
    dprintf(fd, "%c%s\t%s\t%s\t%d%s", item->type, item->display, item->selector, item->host, item->port, CRLF);
}


enum item_types resolve_item(struct dirent * entry){
    enum item_types type;
    switch( entry->d_type ){
        case DT_REG:
            if( str_ends_with(entry->d_name, ".zip") ) {
                type = ITEM_ARCHIVE;
            } else {
                type = ITEM_FILE; // text file?
            }
            break;
        case DT_DIR: type = ITEM_DIR; break;
        default: type = NO_ITEM;
    }
    return type;
}

/** Read directory items
 *  and return array of dirents
 * 
 */

void print_directory(int fd, const char * selector){
    char * dirname = resolve_selector(NULL, selector);
    DIR * dir = opendir(dirname);
    struct dirent * entry = NULL;
    
    if (dir == NULL){
        perror(NULL);
        return;
    }
    
    while( (entry = readdir(dir)) != NULL ){
        if( (*entry).d_name[0] == '.' ) continue; // don't print hidden files/dirs
            
        char type = resolve_item(entry);
        
        char *sel;
        asprintf(&sel, "%s/%s", selector, entry->d_name);
        
        menu_item * item = menu_item_new(type, entry->d_name, sel, DEFAULT_HOST, DEFAULT_PORT);
        print_menu_item(fd, item);
        menu_item_free(item);
        
    }
    
    print_closing(fd);
    if( closedir(dir) != 0 )
        perror(NULL);
    free(dirname);
}

/** Read a file and print it line by line to the server
 *  This functions read the while file at once ... we should do that block by block
 */

void print_file(int fd, const char * selector){
    char * filename = resolve_selector(NULL, selector);
    FILE * file = fopen(filename, "rb");
    
    if (file != NULL) {
        fseek(file, 0L, SEEK_END);     /* Position to end of file */
        long file_len = ftell(file);        /* Get file length */
        rewind(file);
        
        char * lineptr = malloc(file_len);
        if (fread(lineptr, file_len, 1, file) == 1) {
            write(fd, lineptr, file_len);
        }
    
        perror(NULL);
        free(lineptr);
        fclose(file);
    }
    free(filename);
}


/** Print informational message
  */

void print_message(int fd, const char * message){
    dprintf(fd, "i%s%s", message, CRLF);
}


void print_closing(int fd){
    dprintf(fd, ".%s", CRLF);
}

/** Terminate the program by releasing all resources
 */

void main_shutdown(){
    plog("Program ended.");
}