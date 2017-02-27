//
// Created by herve on 17-2-22.
//


#include <sys/time.h>
#include "timer.h"

static int timer_comp(void *ti, void *tj) {
    yas_timer_node *timeri = (yas_timer_node *)ti;
    yas_timer_node *timerj = (yas_timer_node *)tj;

    return (timeri->key < timerj->key)? 1: 0;
}

yas_pq_t yas_timer;
size_t yas_current_msec;

static void yas_time_update() {
    // there is only one thread calling yas_time_update, no need to lock?
    struct timeval tv;
    int rc;

    rc = gettimeofday(&tv, NULL);
    check(rc == 0, "yas_time_update: gettimeofday error");

    yas_current_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    debug("in yas_time_update, time = %zu", yas_current_msec);
}


int yas_timer_init() {
    int rc;
    rc = yas_pq_init(&yas_timer, timer_comp, YAS_PQ_DEFAULT_SIZE);
    check(rc == YAS_OK, "yas_pq_init error");

    yas_time_update();
    return YAS_OK;
}

int yas_find_timer() {
    yas_timer_node *timer_node;
    int time = YAS_TIMER_INFINITE;
    int rc;

    while (!yas_pq_is_empty(&yas_timer)) {
        debug("yas_find_timer");
        yas_time_update();
        timer_node = (yas_timer_node *)yas_pq_min(&yas_timer);
        check(timer_node != NULL, "yas_pq_min error");

        if (timer_node->deleted) {
            rc = yas_pq_delmin(&yas_timer);
            check(rc == 0, "yas_pq_delmin");
            free(timer_node);
            continue;
        }

        time = (int) (timer_node->key - yas_current_msec);
        debug("in yas_find_timer, key = %zu, cur = %zu",
              timer_node->key,
              yas_current_msec);
        time = (time > 0? time: 0);
        break;
    }

    return time;
}

void yas_handle_expire_timers() {
    debug("in yas_handle_expire_timers");
    yas_timer_node *timer_node;
    int rc;

    while (!yas_pq_is_empty(&yas_timer)) {
        debug("yas_handle_expire_timers, size = %zu", yas_pq_size(&yas_timer));
        yas_time_update();
        timer_node = (yas_timer_node *)yas_pq_min(&yas_timer);
        check(timer_node != NULL, "yas_pq_min error");

        if (timer_node->deleted) {
            rc = yas_pq_delmin(&yas_timer);
            check(rc == 0, "yas_handle_expire_timers: yas_pq_delmin error");
            free(timer_node);
            continue;
        }

        if (timer_node->key > yas_current_msec) {
            return;
        }

        if (timer_node->handler) {
            timer_node->handler(timer_node->rq);
        }
        rc = yas_pq_delmin(&yas_timer);
        check(rc == 0, "yas_handle_expire_timers: yas_pq_delmin error");
        free(timer_node);
    }
}

void yas_add_timer(yas_http_request_t *rq, size_t timeout, timer_handler_pt handler) {
    int rc;
    yas_timer_node *timer_node = (yas_timer_node *)malloc(sizeof(yas_timer_node));
    check(timer_node != NULL, "yas_add_timer: malloc error");

    yas_time_update();
    rq->timer = timer_node;
    timer_node->key = yas_current_msec + timeout;
    debug("in yas_add_timer, key = %zu", timer_node->key);
    timer_node->deleted = 0;
    timer_node->handler = handler;
    timer_node->rq = rq;

    rc = yas_pq_insert(&yas_timer, timer_node);
    check(rc == 0, "yas_add_timer: yas_pq_insert error");
}

void yas_del_timer(yas_http_request_t *rq) {
    debug("in yas_del_timer");
    yas_time_update();
    yas_timer_node *timer_node = rq->timer;
    check(timer_node != NULL, "yas_del_timer: rq->timer is NULL");

    timer_node->deleted = 1;
}