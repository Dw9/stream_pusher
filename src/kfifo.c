/*
 * A simple kernel FIFO implementation.
 *
 * Copyright (C) 2004 Stelian Pop <stelian@popies.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "kfifo.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#define kmalloc(size, mask) malloc(size)
#define kfree(ptr) free(ptr)
#define EXPORT_SYMBOL(sym)
#define BUG_ON(cond) assert(!(cond))
#define ERR_PTR(val) NULL
#define IS_ERR(val) (!(val))
#define min(x,y) ({ \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        (void) (&_x == &_y);            \
        _x < _y ? _x : _y; })


#if defined(__GNUC__) || defined(__x86_64__)
#define TPOOL_COMPILER_BARRIER() __asm__ __volatile("" : : : "memory")

static inline void FullMemoryBarrier()
{
   // __asm__ __volatile__("mfence": : : "memory");
}

#define smp_mb() FullMemoryBarrier()
#define smp_rmb() TPOOL_COMPILER_BARRIER()
#define smp_wmb() TPOOL_COMPILER_BARRIER()

#else
#error "smp_mb has not been implemented for this architecture."
#endif



static __inline__ int fls(int x)
{
        int r = 32;

        if (!x)
                return 0;
        if (!(x & 0xffff0000u)) {
                x <<= 16;
                r -= 16;
        }
        if (!(x & 0xff000000u)) {
                x <<= 8;
                r -= 8;
        }
        if (!(x & 0xf0000000u)) {
                x <<= 4;
                r -= 4;
        }
        if (!(x & 0xc0000000u)) {
                x <<= 2;
                r -= 2;
        }
        if (!(x & 0x80000000u)) {
                x <<= 1;
                r -= 1;
        }
        return r;
}

#ifdef __GNUC__
//#define __attribute_const__     __attribute__((__const__))
#endif

static inline unsigned long __attribute_const__ roundup_pow_of_two(unsigned long x)
{
        return (1UL << fls(x - 1));
}


/*
 * kfifo_init - allocates a new FIFO using a preallocated buffer
 * @buffer: the preallocated buffer to be used.
 * @size: the size of the internal buffer, this have to be a power of 2.
 * @gfp_mask: get_free_pages mask, passed to kmalloc()
 * @lock: the lock to be used to protect the fifo buffer
 *
 * Do NOT pass the kfifo to kfifo_free() after use ! Simply free the
 * struct kfifo with kfree().
 */
struct kfifo *kfifo_init(unsigned char *buffer, unsigned int size,
                         int gfp_mask, spinlock_t *lock)
{
        struct kfifo *fifo;

        /* size must be a power of 2 */
        BUG_ON(size & (size - 1));

        fifo = (struct kfifo *)kmalloc(sizeof(struct kfifo), gfp_mask);
        if (!fifo)
                return ERR_PTR(-ENOMEM);

        fifo->buffer = buffer;
        fifo->size = size;
        fifo->in = fifo->out = 0;
        fifo->lock = lock;

        return fifo;
}
EXPORT_SYMBOL(kfifo_init);

/*
 * kfifo_alloc - allocates a new FIFO and its internal buffer
 * @size: the size of the internal buffer to be allocated.
 * @gfp_mask: get_free_pages mask, passed to kmalloc()
 * @lock: the lock to be used to protect the fifo buffer
 *
 * The size will be rounded-up to a power of 2.
 */
struct kfifo *kfifo_alloc(unsigned int size, int gfp_mask, spinlock_t *lock)
{
        unsigned char *buffer;
        struct kfifo *ret;

        /*
         * round up to the next power of 2, since our 'let the indices
         * wrap' tachnique works only in this case.
         */
        if (size & (size - 1)) {
                BUG_ON(size > 0x80000000);
                size = roundup_pow_of_two(size);
        }

        buffer = (unsigned char*)kmalloc(size, gfp_mask);
        if (!buffer)
                return ERR_PTR(-ENOMEM);

        ret = kfifo_init(buffer, size, gfp_mask, lock);

        if (IS_ERR(ret))
                kfree(buffer);

        return ret;
}
EXPORT_SYMBOL(kfifo_alloc);

/*
 * kfifo_free - frees the FIFO
 * @fifo: the fifo to be freed.
 */
void kfifo_free(struct kfifo *fifo)
{
        kfree(fifo->buffer);
        kfree(fifo);
}
EXPORT_SYMBOL(kfifo_free);

/*
 * __kfifo_put - puts some data into the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @buffer: the data to be added.
 * @len: the length of the data to be added.
 *
 * This function copies at most 'len' bytes from the 'buffer' into
 * the FIFO depending on the free space, and returns the number of
 * bytes copied.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
unsigned int __kfifo_put(struct kfifo *fifo,
                         unsigned char *buffer, unsigned int len)
{
        unsigned int l;

        len = min(len, fifo->size - fifo->in + fifo->out);

        smp_mb();

        /* first put the data starting from fifo->in to buffer end */
        l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
        memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

        /* then put the rest (if any) at the beginning of the buffer */
        memcpy(fifo->buffer, buffer + l, len - l);

        smp_wmb();

        fifo->in += len;

        return len;
}
EXPORT_SYMBOL(__kfifo_put);

unsigned int __kfifo_put_chunk(struct kfifo *fifo,
                         unsigned char *buffer, unsigned int len)
{
        unsigned int l;
        // 放不下，直接返回0
        if (len > (fifo->size - fifo->in + fifo->out))
                return 0;

        smp_mb();

        /* first put the data starting from fifo->in to buffer end */
        l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
        memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

        /* then put the rest (if any) at the beginning of the buffer */
        memcpy(fifo->buffer, buffer + l, len - l);

        smp_wmb();

        fifo->in += len;

        return len;
}
EXPORT_SYMBOL(__kfifo_put_chunk);

/*
 * __kfifo_get - gets some data from the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @buffer: where the data must be copied.
 * @len: the size of the destination buffer.
 *
 * This function copies at most 'len' bytes from the FIFO into the
 * 'buffer' and returns the number of copied bytes.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
unsigned int __kfifo_get(struct kfifo *fifo,
                         unsigned char *buffer, unsigned int len)
{
        unsigned int l;

        len = min(len, fifo->in - fifo->out);

        smp_rmb();

        /* first get the data from fifo->out until the end of the buffer */
        l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
        memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

        /* then get the rest (if any) from the beginning of the buffer */
        memcpy(buffer + l, fifo->buffer, len - l);

        smp_mb();

        fifo->out += len;

        return len;
}
EXPORT_SYMBOL(__kfifo_get);

unsigned int __kfifo_get_chunk(struct kfifo *fifo,
                         unsigned char *buffer, unsigned int len)
{
        unsigned int l;
        if (len > (fifo->in - fifo->out))
                return 0;

        smp_rmb();

        /* first get the data from fifo->out until the end of the buffer */
        l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
        memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

        /* then get the rest (if any) from the beginning of the buffer */
        memcpy(buffer + l, fifo->buffer, len - l);

        smp_mb();

        fifo->out += len;

        return len;
}
EXPORT_SYMBOL(__kfifo_get_chunk);
