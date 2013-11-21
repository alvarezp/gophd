#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <stdlib.h>
#include <string.h>
#include "types.h"

#define ITEM_DELIM '\t';

struct menu_item * menu_item_new(const char type, const char * display, const char * selector, const char * host, unsigned int port){
    menu_item * item = calloc(1, sizeof(menu_item));
    
    item->type = type;
    item->display = display ? strdup(display) : "";
    item->selector = selector ? strdup(selector) : "";
    item->host = host ? strdup(host) : "";
    item->port = port;
    return item;
}


struct menu_item * menu_item_parse( char type, char * origline, char * default_host, unsigned int default_port ){
    char * display = NULL;
    char * selector = NULL;
    char * host = NULL;
    unsigned int port = 0;
    unsigned int tokenc = 0;
    char * token = NULL;
    char * line = strdup(origline);
    menu_item * item;
    
    // Cut out trailing newline
    size_t line_len = strlen(line);
    if ( line[line_len-1] == '\n' ) line[line_len-1] = '\0';
    
    token = strtok( line, "\t" );
    while( token != NULL ){
        tokenc++;
        if( !display ) display = token;
        else if( !selector ) selector = token;
        else if( !host ) host = token;
        else if( !port ) port = atoi(token); 
        token = strtok( NULL, "\t" );
    } 
    if (type != ITEM_INFO && type != ERROR){
        if ( !host ) host = default_host;
        if ( !port ) port = default_port;
    }
    item = menu_item_new( type, display, selector, host, port );
    free(line);
    return item;
}



void menu_item_free(menu_item * item){
    free(item->display);
    free(item->selector);
    free(item->host);
    free(item);
}
