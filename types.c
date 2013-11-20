#include <stdlib.h>
#include <string.h>
#include "types.h"

#define ITEM_DELIM '\t';

struct menu_item * menu_item_new(const char type, const char * display, const char * selector, const char * host, unsigned int port){
    menu_item * item = calloc(1, sizeof(menu_item));
    
    item->type = type;
    item->display = strdup(display);
    item->selector = strdup(selector);
    item->host = strdup(host);
    item->port = port;
    item->delimiter = ITEM_DELIM;
    return item;
}


void menu_item_free(menu_item * item){
    free(item->display);
    free(item->selector);
    free(item->host);
    free(item);
}