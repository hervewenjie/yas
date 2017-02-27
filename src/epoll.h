//
// Created by herve on 17-2-22.
//

#ifndef YAS_EPOLL_H
#define YAS_EPOLL_H

#include <sys/epoll.h>

#define MAXEVENTS 1024

int yas_epoll_create(int flags);
void yas_epoll_add(int epfd, int fs, struct epoll_event *event);
void yas_epoll_mod(int epfd, int fs, struct epoll_event *event);
void yas_epoll_del(int epfd, int fs, struct epoll_event *event);
int yas_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#endif //YAS_EPOLL_H
