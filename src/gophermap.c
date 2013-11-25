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
#include <stdbool.h>
#include <string.h>
#include "types.h"
#include "utils.h"

#define GM_MAX_LINESIZE 200
#define GM_MIN_LINESIZE 5


/** Checks if given line is a valid gopher line:
 *  1. Does begin with a valid ASCII character 
 *  2. AND has at least one TAB
 *  3. AND Is at least 5 chars long
 *  4. OR begins with 'i'
 */

static bool is_valid_gopher_line( const char * line )
{
	return ( strlen(line) >= GM_MIN_LINESIZE && (
		( line[0] >= '0' && line[0] <= '9' ) ||
		( line[0] >= 'A' && line[0] <= 'Z' ) ||
		( line[0] >= 'a' && line[0] <= 'z' ) ) &&
		( strchr(line, '\t') || line[0] == 'i' ) );
}


/** parse a gophermap file and return an array of menu_items
 *  return number if menu_items in array, 0 = for none or failure
 *
 * 1. Open File
 * 2. Loop over file and construct a menu_item per line
 * 3. If Line does not conform to a Gopher item (valid item plus tabs), it is converted to a INFO item
 * 4. If HOST oder PORT is missing, add default host and port
 */

unsigned int parse_gophermap( const char * filepath, menu_item ** items, char * def_host, unsigned int def_port )
{
    char * line = NULL;
    size_t maxlen = 0;
    unsigned int linec = 0;
    FILE * mapf = fopen( filepath, "r" );
    if( !mapf ) return 0; // Error, return zero items
    for( linec = 0; getline( &line, &maxlen, mapf ) != -1; linec++ ){
        if ( is_valid_gopher_line(line) ){
            items[linec] = menu_item_parse( line[0], line+1, def_host, def_port );
        } else {
            items[linec] = menu_item_parse( 'i', line, def_host, def_port );
        }
    }

    fclose(mapf);
    return linec;
}


/*

int main()
{
	struct menu_item ** items;
	parse_gophermap("../examples/Gophermap", items);
}

*/