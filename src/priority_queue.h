//
// Created by herve on 17-2-22.
//

#ifndef YAS_PRIORITY_QUEUE_H
#define YAS_PRIORITY_QUEUE_H

#include "dbg.h"
#include "error.h"

#define YAS_PQ_DEFAULT_SIZE 10

typedef int (*yas_pq_comparator_pt)(void *pi, void *pj);

typedef struct {
    void **pq;
    size_t nalloc;
    size_t size;
    yas_pq_comparator_pt comp;
} yas_pq_t;

int yas_pq_init(yas_pq_t *yas_pq, yas_pq_comparator_pt comp, size_t size);
int yas_pq_is_empty(yas_pq_t *yas_pq);
size_t yas_pq_size(yas_pq_t *yas_pq);
void *yas_pq_min(yas_pq_t *yas_pq);
int yas_pq_delmin(yas_pq_t *yas_pq);
int yas_pq_insert(yas_pq_t *yas_pq, void *item);

int yas_pq_sink(yas_pq_t *yas_pq, size_t i);

#endif //YAS_PRIORITY_QUEUE_H
