#include <stdio.h>
#include <string.h>
#include "utils.h"

int str_ends_with(const char * str, const char * ending){
    char * ptr = strcasestr(str, ending);
    if( ptr == NULL ) return 0;
    return ( ptr == str+strlen(str)-strlen(ptr) );
}