#ifndef gophd_utils_h
#define gophd_utils_h

#define plog(...)                                   \
        fprintf(stderr, __VA_ARGS__);   \
        fprintf(stderr, "\n");

int str_ends_with(const char * str, const char * ending);

#endif
