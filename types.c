#include <stdlib.h>
#include <string.h>
#include "types.h"

#define ITEM_DELIM '\t';

struct menu_item * menu_item_new(const char type, const char * display, const char * selector, const char * host, unsigned int port){
    menu_item * item = calloc(1, sizeof(menu_item));
    
    item->type = type;
    strcpy(item->display, display);
    strcpy(item->selector, selector);
    strcpy(item->host, host);
    item->port = port;
    item->delimiter = ITEM_DELIM;
    return item;
}


void menu_item_free(menu_item * item){
    free(item);
}