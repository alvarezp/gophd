#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "utils.h"

int str_ends_with(const char * str, const char * ending){
    char * ptr = strcasestr(str, ending);
    if( ptr == NULL ) return 0;
    return ( ptr == str+strlen(str)-strlen(ptr) );
}

int exists(const char *fname)
{
    printf("Checking for %s\n", fname);
    FILE *file;
    if ( (file = fopen(fname, "r")) )
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int is_file(char * path){
    struct stat * fstat = malloc(sizeof(struct stat));
    int success = stat(path, fstat) == 0 && S_ISREG(fstat->st_mode);
    free(fstat);
    return success;
}

int is_dir(char * path){
    struct stat * fstat = malloc(sizeof(struct stat));
    int success = stat(path, fstat) == 0 && S_ISDIR(fstat->st_mode);
    free(fstat);
    return success;
}