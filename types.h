#ifndef gophd_types_h
#define gophd_types_h

enum item_types {
    ITEM_FILE='0',
    ITEM_DIR='1',
    ERROR='3',
    ITEM_ARCHIVE='5',
    ITEM_BINARY='9',
    ITEM_IMAGE='I',
    ITEM_INFO='i',
    ITEM_GIF='g',
    ITEM_PNG='p',
    ITEM_PDF='d',
    NO_ITEM=0
};


struct menu_item {
    char type;
    char display[80];
    char selector[256];
    char host[256];
    unsigned int port;
    char delimiter;
};

struct request_t {
    int fd;
    char * selector;
    size_t selector_len;
    char * path;
    size_t path_len;
};

typedef struct menu_item menu_item;
typedef char t_item;

struct menu_item * menu_item_new(const char type, const char * display, const char * selector, const char * host, unsigned int port);void menu_item_free(menu_item * item);

#endif
