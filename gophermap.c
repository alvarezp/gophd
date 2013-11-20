/** gophermap.
 *
 *  Library to support parsing of a gophermap file
 *
 *  API:
 *  - Open a gophermap file and return a FILE * to it
 *  - Parse a gophermap FILE and return an array of menu_items_types
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "types.h"

unsigned int parse_gophermap( const char * filepath, struct menu_item ** items_r ){
    
    /*FILE * mapf = fopen( filepath, "r" );
    
    if( mapf ){
        
        fclose(mapf);
    }
    */
    
    struct menu_item ** items = malloc( sizeof(struct menu_item)*100 );
    
    items[0] = menu_item_new( ITEM_INFO, "Hello Display", "selector", "host", 70 );
    items_r = items;
    
    return 1;
}
