//
// Created by herve on 17-2-22.
//


#include "priority_queue.h"

int yas_pq_init(yas_pq_t *yas_pq, yas_pq_comparator_pt comp, size_t size) {
    yas_pq->pq = (void **)malloc(sizeof(void *) * (size+1));
    if (!yas_pq->pq) {
        log_err("yas_pq_init: malloc failed");
        return -1;
    }

    yas_pq->nalloc = 0;
    yas_pq->size = size + 1;
    yas_pq->comp = comp;

    return YAS_OK;
}

int yas_pq_is_empty(yas_pq_t *yas_pq) {
    return (yas_pq->nalloc == 0)? 1: 0;
}

size_t yas_pq_size(yas_pq_t *yas_pq) {
    return yas_pq->nalloc;
}

void *yas_pq_min(yas_pq_t *yas_pq) {
    if (yas_pq_is_empty(yas_pq)) {
        return NULL;
    }

    return yas_pq->pq[1];
}

static int resize(yas_pq_t *yas_pq, size_t new_size) {
    if (new_size <= yas_pq->nalloc) {
        log_err("resize: new_size to small");
        return -1;
    }

    void **new_ptr = (void **)malloc(sizeof(void *) * new_size);
    if (!new_ptr) {
        log_err("resize: malloc failed");
        return -1;
    }

    memcpy(new_ptr, yas_pq->pq, sizeof(void *) * (yas_pq->nalloc + 1));
    free(yas_pq->pq);
    yas_pq->pq = new_ptr;
    yas_pq->size = new_size;
    return YAS_OK;
}

static void exch(yas_pq_t *yas_pq, size_t i, size_t j) {
    void *tmp = yas_pq->pq[i];
    yas_pq->pq[i] = yas_pq->pq[j];
    yas_pq->pq[j] = tmp;
}

static void swim(yas_pq_t *yas_pq, size_t k) {
    while (k > 1 && yas_pq->comp(yas_pq->pq[k], yas_pq->pq[k/2])) {
        exch(yas_pq, k, k/2);
        k /= 2;
    }
}

static size_t sink(yas_pq_t *yas_pq, size_t k) {
    size_t j;
    size_t nalloc = yas_pq->nalloc;

    while (2*k <= nalloc) {
        j = 2*k;
        if (j < nalloc && yas_pq->comp(yas_pq->pq[j+1], yas_pq->pq[j])) j++;
        if (!yas_pq->comp(yas_pq->pq[j], yas_pq->pq[k])) break;
        exch(yas_pq, j, k);
        k = j;
    }

    return k;
}

int yas_pq_delmin(yas_pq_t *yas_pq) {
    if (yas_pq_is_empty(yas_pq)) {
        return YAS_OK;
    }

    exch(yas_pq, 1, yas_pq->nalloc);
    yas_pq->nalloc--;
    sink(yas_pq, 1);
    if (yas_pq->nalloc > 0 && yas_pq->nalloc <= (yas_pq->size - 1)/4) {
        if (resize(yas_pq, yas_pq->size / 2) < 0) {
            return -1;
        }
    }

    return YAS_OK;
}

int yas_pq_insert(yas_pq_t *yas_pq, void *item) {
    if (yas_pq->nalloc + 1 == yas_pq->size) {
        if (resize(yas_pq, yas_pq->size * 2) < 0) {
            return -1;
        }
    }

    yas_pq->pq[++yas_pq->nalloc] = item;
    swim(yas_pq, yas_pq->nalloc);

    return YAS_OK;
}

int yas_pq_sink(yas_pq_t *yas_pq, size_t i) {
    return sink(yas_pq, i);
}