#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * SPSC
 */

struct fifo8 {
	uint8_t *buf;
	unsigned long size;
	atomic_ulong head;
	atomic_ulong tail;
};

extern void fifo8_reset(struct fifo8 *fifo);
extern struct fifo8 *fifo8_new(unsigned long items);
extern bool fifo8_push(struct fifo8 *fifo, uint8_t in);
extern bool fifo8_pop(struct fifo8 *fifo, uint8_t *out);
extern unsigned long fifo8_num_used(struct fifo8 *fifo);
extern unsigned long fifo8_num_free(struct fifo8 *fifo);
extern bool fifo8_is_full(struct fifo8 *fifo);
extern bool fifo8_is_empty(struct fifo8 *fifo);

struct fifo16 {
	uint16_t *buf;
	unsigned long size;
	atomic_ulong head;
	atomic_ulong tail;
};

extern void fifo16_reset(struct fifo16 *fifo);
extern struct fifo16 *fifo16_new(unsigned long items);
extern bool fifo16_push(struct fifo16 *fifo, uint16_t in);
extern bool fifo16_pop(struct fifo16 *fifo, uint16_t *out);
extern unsigned long fifo16_num_used(struct fifo16 *fifo);
extern unsigned long fifo16_num_free(struct fifo16 *fifo);
extern bool fifo16_is_full(struct fifo16 *fifo);
extern bool fifo16_is_empty(struct fifo16 *fifo);

struct fifo32 {
	uint32_t *buf;
	unsigned long size;
	atomic_ulong head;
	atomic_ulong tail;
};

extern void fifo32_reset(struct fifo32 *fifo);
extern struct fifo32 *fifo32_new(unsigned long items);
extern bool fifo32_push(struct fifo32 *fifo, uint32_t in);
extern bool fifo32_pop(struct fifo32 *fifo, uint32_t *out);
extern unsigned long fifo32_num_used(struct fifo32 *fifo);
extern unsigned long fifo32_num_free(struct fifo32 *fifo);
extern bool fifo32_is_full(struct fifo32 *fifo);
extern bool fifo32_is_empty(struct fifo32 *fifo);

struct fifo64 {
	uint64_t *buf;
	unsigned long size;
	atomic_ulong head;
	atomic_ulong tail;
};

extern void fifo64_reset(struct fifo64 *fifo);
extern struct fifo64 *fifo64_new(unsigned long items);
extern bool fifo64_push(struct fifo64 *fifo, uint64_t in);
extern bool fifo64_pop(struct fifo64 *fifo, uint64_t *out);
extern unsigned long fifo64_num_used(struct fifo64 *fifo);
extern unsigned long fifo64_num_free(struct fifo64 *fifo);
extern bool fifo64_is_full(struct fifo64 *fifo);
extern bool fifo64_is_empty(struct fifo64 *fifo);
