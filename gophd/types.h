#ifndef gophd_types_h
#define gophd_types_h

enum item_types { ITEM_FILE='0', ITEM_DIR='1', ERROR='3', ITEM_BINARY='9', ITEM_IMAGE='I', ITEM_INFO='i' };


struct menu_item {
    char type;
    char display[80];
    char selector[256];
    char host[256];
    unsigned int port;
    char delimiter;
};

typedef struct menu_item menu_item;

struct menu_item * menu_item_new(const char type, const char * display, const char * selector, const char * host, unsigned int port);
void menu_item_free(menu_item * item);

#endif
