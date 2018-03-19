#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

typedef struct BufHdr {
    uint64_t len;
    uint64_t cap;
    char buf[0];
} BufHdr;

#define buf__hdr(b) ( (BufHdr *)(((char*)(b)) - offsetof(BufHdr, buf)) )
#define buf__fits(b, n) ( buf_len((b)) + (n) <= buf_cap((b)) )
#define buf__fit(b, n) ( buf__fits((b), (n)) ? 0 : ((b) = buf__grow((b), buf_len((b)) + (n), sizeof(*(b)))) )

#define buf_len(b) ( (b) ? buf__hdr((b))->len : 0 )
#define buf_cap(b) ( (b) ? buf__hdr((b))->cap : 0 )
#define buf_reserve(b, c) ( buf_cap(b) <= (c) ? ((b) = buf__reserve((b), (c), sizeof(*(b)))) : 0 )
#define buf_free(b) ( free(buf__hdr(b)), (b) = NULL )
#define buf_push(b, v) ( buf__fit((b), 1), (b)[buf__hdr(b)->len++] = (v) )

void *buf__reserve(void *buf, size_t new_cap, size_t elem_size) {
    assert(buf_cap(buf) <= new_cap);
    size_t new_size = offsetof(BufHdr, buf) + new_cap * elem_size;
    BufHdr* hdr = buf__hdr(buf);
    if (buf) {
        hdr = realloc(hdr, new_size);
    } else {
        hdr = malloc(new_size);
        hdr->len = 0;
    }
    hdr->cap = new_cap;
    return hdr->buf;
}

void *buf__grow(void *buf, size_t new_len, size_t elem_size) {
    size_t new_cap = (buf_cap(buf) * 2) + 1;
    assert(new_len <= new_cap);
    return buf__reserve(buf, new_cap, elem_size);
}

int main(int argc, char** argv) {

    int* int_buf = NULL;
    assert(buf_len(int_buf) == 0);
    assert(buf_cap(int_buf) == 0);

    enum { N = 1024 }; 
    for (size_t i = 0; i < N; i++) {
        buf_push(int_buf, i);
    }
    for (size_t i = 0; i < buf_len(int_buf); i++) {
        printf("%d\n", int_buf[i]);
    }
    buf_free(int_buf);
    float* float_buf = NULL;
    buf_reserve(float_buf, 1024);
    size_t cap = buf_cap(float_buf);
    assert(cap == 1024);
    assert(buf_len(float_buf) == 0);
    return 0;
}
