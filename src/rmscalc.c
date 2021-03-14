#include "rmscalc.h"
#include <assert.h>

/*
 * we use a buffer to store the squared samples. the buffer is as big as to
 * save all the samples needed for one period of the signal at the lowest freq.
 */
struct rbuf
{
    int32_t *buf;
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t tsize;
};

#define RINGBUF_CREATE(name, size32)     \
    static int32_t _rbuf_##name[size32]; \
    static struct rbuf name = {          \
        .buf = _rbuf_##name,             \
        .head = 0,                       \
        .tail = 0,                       \
        .size = size32,                  \
        .tsize = size32,                 \
    };

static inline void ringbuf_push(struct rbuf *buf, uint32_t val)
{
    buf->buf[buf->tail] = val;
    buf->tail++;
    if (buf->tail > buf->size)
    {
        buf->tail = 0;
    }
}

static inline uint32_t ringbuf_pop(struct rbuf *buf)
{
    uint32_t idx = buf->head;
    buf->head++;
    if (buf->head > buf->size)
    {
        buf->head = 0;
    }
    return buf->buf[idx];
}

static inline int ringbuf_set_size(struct rbuf *buf, int32_t size)
{
    if (buf->size > buf->tsize)
    {
        return -1;
    }
    buf->size = size;
    return 0;
}

static inline uint32_t ringbuf_pushpop(struct rbuf *buf, int32_t val)
{
    int32_t rval = 0;
    buf->buf[buf->tail] = val;
    buf->tail++;
    if (buf->tail > buf->size)
    {
        buf->tail = 0;
    }
    if (buf->tail == buf->head)
    {
        /* buffer is full, pop the last element */
        rval = buf->buf[buf->head];
        buf->head++;
        if (buf->head > buf->size)
        {
            buf->head = 0;
        }
    }
    return rval;
}

/* computes the integer square root of a int64_t number */
static int32_t isqrt64(int64_t x)
{
    uint64_t c = 0x80000000;
    uint64_t g = 0x80000000;

    for (;;)
    {
        if (g * g > x)
            g ^= c;
        c >>= 1;
        if (c == 0)
            break;
        g |= c;
    }

    return g;
}

uint16_t sqrtd(uint32_t x)
{
    uint32_t tmp;
    uint16_t g;
    uint16_t b;
    uint8_t shft;
    g = 0;
    shft = 15;
    for (b = 0x8000; b; b >>= 1)
    {
        tmp = ((uint32_t)g << 1) + b;
        tmp = tmp << (shft--);
        if (x >= tmp)
        {
            g += b;
            x -= tmp;
        }
    }
    return g;
}

typedef int32_t f32;
typedef int16_t f16;

RINGBUF_CREATE(buf, SAMPLE_RATE / MIN_FREQ + 1);

int16_t rmscalc(int16_t sample, uint16_t freq)
{
    int rval;
    static uint32_t val;
    uint32_t size = (SAMPLE_RATE * 10) / freq;
    rval = ringbuf_set_size(&buf, size);
    assert(rval == 0);
    int32_t sqr = sample * sample;
    val += sqr;
    sqr = ringbuf_pushpop(&buf, sqr);
    val -= sqr;
    return isqrt64(val / size);
}

int16_t rmscalc_sp(int16_t sample, uint16_t freq)
{
    static uint32_t oval = 0;
    static uint32_t aval = 0;
    static uint32_t scnt = 0;

    uint32_t samples = (SAMPLE_RATE * 10) / freq;

    aval += sample * sample;
    scnt++;

    if (scnt >= samples)
    {
        oval = sqrtd(aval / samples);
        aval = 0;
        scnt = 0;
    }

    return oval;
}

/* optimized 1 order frac 32 filter */
struct f16iir_filter_1od
{
    f16 a;
    f16 b[2];
};

/**
 * @brief Delay line for the explicit rms converter
 */
struct f16rms_dl
{
    f16 dx[2];
    f16 dy[2];
};

f16 f16rms_explicit(const struct f16iir_filter_1od *f, struct f16rms_dl *dl, f16 x)
{
    f32 t, a;
    t = x * x >> 15;
    a = t;
    /*
   * first recursive first order filter
   * The filter is calculated with following equation:
   *
   *  y[n] = b[0] * x[n] + dx
   *    dx = b[1] * x[n] - a[0] * dy
   *    dy = y
   */
    a = a * f->b[0] >> 15;
    a = a + (dl->dx[0] * f->b[1] >> 15);
    dl->dx[0] = t;
    a = a + (dl->dy[0] * f->a >> 15);
    dl->dy[0] = a;
    /*
   * This is the second of the cascade of recursive filters
   */
    a = a * f->b[0] >> 15;
    a = a + (dl->dx[1] * f->b[1] >> 15);
    dl->dx[1] = dl->dy[0];
    a = a + (dl->dy[1] * f->a >> 15);
    dl->dy[1] = a;
    /* return the square root of the value */
    return sqrtd(a);
}
