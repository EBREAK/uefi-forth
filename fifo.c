#include "fifo.h"
#include "main.h"

void fifo8_reset(struct fifo8 *fifo)
{
	atomic_store(&fifo->head, 0);
	atomic_store(&fifo->tail, 0);
	atomic_thread_fence(memory_order_seq_cst);
}

struct fifo8 *fifo8_new(unsigned long items)
{
	if (items < 2) {
		return NULL;
	}
	struct fifo8 *fifo;
	UINTN alloc_size = sizeof(struct fifo8);
	alloc_size += sizeof(uint8_t) * items;
	EFI_STATUS Status;
	Status = GST->BootServices->AllocatePool(EfiRuntimeServicesData,
						 alloc_size, (VOID **)&fifo);
	if (EFI_ERROR(Status)) {
		return NULL;
	}
	fifo->buf = ((uint8_t *)fifo) + sizeof(struct fifo8);
	fifo->size = items;
	fifo8_reset(fifo);
	return fifo;
}

bool fifo8_push(struct fifo8 *fifo, uint8_t in)
{
	unsigned long curr_tail;
	curr_tail = atomic_load(&fifo->tail);
	unsigned long next_tail;
	next_tail = (curr_tail + 1) % fifo->size;
	if (next_tail == atomic_load(&fifo->head)) {
		return false;
	}
	fifo->buf[curr_tail] = in;
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store(&fifo->tail, next_tail);
	return true;
}

bool fifo8_pop(struct fifo8 *fifo, uint8_t *out)
{
	unsigned long curr_head;
	curr_head = atomic_load(&fifo->head);
	if (curr_head == atomic_load(&fifo->tail)) {
		return false;
	}
	*out = fifo->buf[curr_head];
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store(&fifo->head, (curr_head + 1) % fifo->size);
	return true;
}

unsigned long fifo8_num_used(struct fifo8 *fifo)
{
	unsigned long head1, head2, tail;
	do {
		head1 = atomic_load(&fifo->head);
		tail = atomic_load(&fifo->tail);
		head2 = atomic_load(&fifo->head);
	} while (head1 != head2);
	unsigned long used = tail + fifo->size - head1;
	if (tail >= head1) {
		used = tail - head1;
	}
	return used;
}

unsigned long fifo8_num_free(struct fifo8 *fifo)
{
	return fifo->size - fifo8_num_used(fifo);
}

bool fifo8_is_full(struct fifo8 *fifo)
{
	return (fifo8_num_free(fifo) == 0);
}

bool fifo8_is_empty(struct fifo8 *fifo)
{
	return (fifo8_num_used(fifo) == 0);
}

void fifo16_reset(struct fifo16 *fifo)
{
	atomic_store(&fifo->head, 0);
	atomic_store(&fifo->tail, 0);
	atomic_thread_fence(memory_order_seq_cst);
}

struct fifo16 *fifo16_new(unsigned long items)
{
	if (items < 2) {
		return NULL;
	}
	struct fifo16 *fifo;
	UINTN alloc_size = sizeof(struct fifo16);
	alloc_size += sizeof(uint16_t) * items;
	EFI_STATUS Status;
	Status = GST->BootServices->AllocatePool(EfiRuntimeServicesData,
						 alloc_size, (VOID **)&fifo);
	if (EFI_ERROR(Status)) {
		return NULL;
	}
	fifo->buf = (uint16_t *)(((uint8_t *)fifo) + sizeof(struct fifo16));
	fifo->size = items;
	fifo16_reset(fifo);
	return fifo;
}

bool fifo16_push(struct fifo16 *fifo, uint16_t in)
{
	unsigned long curr_tail;
	curr_tail = atomic_load(&fifo->tail);
	unsigned long next_tail;
	next_tail = (curr_tail + 1) % fifo->size;
	if (next_tail == atomic_load(&fifo->head)) {
		return false;
	}
	fifo->buf[curr_tail] = in;
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store(&fifo->tail, next_tail);
	return true;
}

bool fifo16_pop(struct fifo16 *fifo, uint16_t *out)
{
	unsigned long curr_head;
	curr_head = atomic_load(&fifo->head);
	if (curr_head == atomic_load(&fifo->tail)) {
		return false;
	}
	*out = fifo->buf[curr_head];
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store(&fifo->head, (curr_head + 1) % fifo->size);
	return true;
}

unsigned long fifo16_num_used(struct fifo16 *fifo)
{
	unsigned long head1, head2, tail;
	do {
		head1 = atomic_load(&fifo->head);
		tail = atomic_load(&fifo->tail);
		head2 = atomic_load(&fifo->head);
	} while (head1 != head2);
	unsigned long used = tail + fifo->size - head1;
	if (tail >= head1) {
		used = tail - head1;
	}
	return used;
}

unsigned long fifo16_num_free(struct fifo16 *fifo)
{
	return fifo->size - fifo16_num_used(fifo);
}

bool fifo16_is_full(struct fifo16 *fifo)
{
	return (fifo16_num_free(fifo) == 0);
}

bool fifo16_is_empty(struct fifo16 *fifo)
{
	return (fifo16_num_used(fifo) == 0);
}

void fifo32_reset(struct fifo32 *fifo)
{
	atomic_store(&fifo->head, 0);
	atomic_store(&fifo->tail, 0);
	atomic_thread_fence(memory_order_seq_cst);
}

struct fifo32 *fifo32_new(unsigned long items)
{
	if (items < 2) {
		return NULL;
	}
	struct fifo32 *fifo;
	UINTN alloc_size = sizeof(struct fifo32);
	alloc_size += sizeof(uint32_t) * items;
	EFI_STATUS Status;
	Status = GST->BootServices->AllocatePool(EfiRuntimeServicesData,
						 alloc_size, (VOID **)&fifo);
	if (EFI_ERROR(Status)) {
		return NULL;
	}
	fifo->buf = (uint32_t *)(((uint8_t *)fifo) + sizeof(struct fifo32));
	fifo->size = items;
	fifo32_reset(fifo);
	return fifo;
}

bool fifo32_push(struct fifo32 *fifo, uint32_t in)
{
	unsigned long curr_tail;
	curr_tail = atomic_load(&fifo->tail);
	unsigned long next_tail;
	next_tail = (curr_tail + 1) % fifo->size;
	if (next_tail == atomic_load(&fifo->head)) {
		return false;
	}
	fifo->buf[curr_tail] = in;
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store(&fifo->tail, next_tail);
	return true;
}

bool fifo32_pop(struct fifo32 *fifo, uint32_t *out)
{
	unsigned long curr_head;
	curr_head = atomic_load(&fifo->head);
	if (curr_head == atomic_load(&fifo->tail)) {
		return false;
	}
	*out = fifo->buf[curr_head];
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store(&fifo->head, (curr_head + 1) % fifo->size);
	return true;
}

unsigned long fifo32_num_used(struct fifo32 *fifo)
{
	unsigned long head1, head2, tail;
	do {
		head1 = atomic_load(&fifo->head);
		tail = atomic_load(&fifo->tail);
		head2 = atomic_load(&fifo->head);
	} while (head1 != head2);
	unsigned long used = tail + fifo->size - head1;
	if (tail >= head1) {
		used = tail - head1;
	}
	return used;
}

unsigned long fifo32_num_free(struct fifo32 *fifo)
{
	return fifo->size - fifo32_num_used(fifo);
}

bool fifo32_is_full(struct fifo32 *fifo)
{
	return (fifo32_num_free(fifo) == 0);
}

bool fifo32_is_empty(struct fifo32 *fifo)
{
	return (fifo32_num_used(fifo) == 0);
}

void fifo64_reset(struct fifo64 *fifo)
{
	atomic_store(&fifo->head, 0);
	atomic_store(&fifo->tail, 0);
	atomic_thread_fence(memory_order_seq_cst);
}

struct fifo64 *fifo64_new(unsigned long items)
{
	if (items < 2) {
		return NULL;
	}
	struct fifo64 *fifo;
	UINTN alloc_size = sizeof(struct fifo64);
	alloc_size += sizeof(uint64_t) * items;
	EFI_STATUS Status;
	Status = GST->BootServices->AllocatePool(EfiRuntimeServicesData,
						 alloc_size, (VOID **)&fifo);
	if (EFI_ERROR(Status)) {
		return NULL;
	}
	fifo->buf = (uint64_t *)(((uint8_t *)fifo) + sizeof(struct fifo64));
	fifo->size = items;
	fifo64_reset(fifo);
	return fifo;
}

bool fifo64_push(struct fifo64 *fifo, uint64_t in)
{
	unsigned long curr_tail;
	curr_tail = atomic_load(&fifo->tail);
	unsigned long next_tail;
	next_tail = (curr_tail + 1) % fifo->size;
	if (next_tail == atomic_load(&fifo->head)) {
		return false;
	}
	fifo->buf[curr_tail] = in;
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store(&fifo->tail, next_tail);
	return true;
}

bool fifo64_pop(struct fifo64 *fifo, uint64_t *out)
{
	unsigned long curr_head;
	curr_head = atomic_load(&fifo->head);
	if (curr_head == atomic_load(&fifo->tail)) {
		return false;
	}
	*out = fifo->buf[curr_head];
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store(&fifo->head, (curr_head + 1) % fifo->size);
	return true;
}

unsigned long fifo64_num_used(struct fifo64 *fifo)
{
	unsigned long head1, head2, tail;
	do {
		head1 = atomic_load(&fifo->head);
		tail = atomic_load(&fifo->tail);
		head2 = atomic_load(&fifo->head);
	} while (head1 != head2);
	unsigned long used = tail + fifo->size - head1;
	if (tail >= head1) {
		used = tail - head1;
	}
	return used;
}

unsigned long fifo64_num_free(struct fifo64 *fifo)
{
	return fifo->size - fifo64_num_used(fifo);
}

bool fifo64_is_full(struct fifo64 *fifo)
{
	return (fifo64_num_free(fifo) == 0);
}

bool fifo64_is_empty(struct fifo64 *fifo)
{
	return (fifo64_num_used(fifo) == 0);
}
