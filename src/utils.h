#ifndef gophd_utils_h
#define gophd_utils_h

#define plog(...)                                   \
        fprintf(stderr, __VA_ARGS__);   \
        fprintf(stderr, "\n");

int str_ends_with(const char * str, const char * ending);
int exists(const char *fname);
int is_file(char * path);
int is_dir(char * path);

#endif
