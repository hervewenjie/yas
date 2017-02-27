//
// Created by herve on 17-2-22.
//

#ifndef YAS_TIMER_H
#define YAS_TIMER_H

#include "priority_queue.h"
#include "http_request.h"

#define YAS_TIMER_INFINITE -1
#define TIMEOUT_DEFAULT 500     /* ms */

typedef int (*timer_handler_pt)(yas_http_request_t *rq);

typedef struct yas_timer_node_s{
    size_t key;
    int deleted;    /* if remote client close the socket first, set deleted to 1 */
    timer_handler_pt handler;
    yas_http_request_t *rq;
} yas_timer_node;

int yas_timer_init();
int yas_find_timer();
void yas_handle_expire_timers();

extern yas_pq_t yas_timer;
extern size_t yas_current_msec;

void yas_add_timer(yas_http_request_t *rq, size_t timeout, timer_handler_pt handler);
void yas_del_timer(yas_http_request_t *rq);

#endif //YAS_TIMER_H
