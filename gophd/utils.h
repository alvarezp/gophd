#ifndef gophd_utils_h
#define gophd_utils_h

#define plog(...)                                   \
        fprintf(stderr, __VA_ARGS__);   \
        fprintf(stderr, "\n");

//void plog(const char * msg);

#endif
