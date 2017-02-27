//
// Created by herve on 17-2-22.
//

#ifndef YAS_UTIL_H
#define YAS_UTIL_H

// max number of listen queue
#define LISTENQ     1024

#define BUFLEN      8192

#define DELIM       "="

#define ZV_CONF_OK      0
#define ZV_CONF_ERROR   100

#define MIN(a,b) ((a) < (b) ? (a) : (b))

struct yas_conf_s {
    void *root;
    int port;
    int thread_num;
};

typedef struct yas_conf_s yas_conf_t;

int open_listenfd(int port);
int make_socket_non_blocking(int fd);

int read_conf(char *filename, yas_conf_t *cf, char *buf, int len);

#endif //YAS_UTIL_H
