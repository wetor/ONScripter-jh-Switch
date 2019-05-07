/*
 * Ringbuffer
 *
 * Copyright (c) 2017, Tuomas Virtanen
 * license: MIT; see LICENSE for details.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "kitchensink/internal/utils/kitringbuffer.h"

/**
  * Creates a new ringbuffer with the given size.
  * @param size Size for the new ringbuffer
  * @return Ringbuffer handle
  */
Kit_RingBuffer* Kit_CreateRingBuffer(unsigned int size) {
    Kit_RingBuffer *rb = calloc(1, sizeof(Kit_RingBuffer));
    if(rb == NULL) return NULL;
    rb->size = size;
    rb->data = malloc(size);
    if(rb->data == NULL) {
        free(rb);
        return NULL;
    }
    return rb;
}

/**
  * Deletes the given ringbuffer.
  * @param rb Ringbuffer to be deleted.
  */
void Kit_DestroyRingBuffer(Kit_RingBuffer* rb) {
    if(rb == NULL) return;
    free(rb->data);
    free(rb);
}

/**
  * Writes to the given ringbuffer. If given length is larger than the amount
  * the ringbuffer can fit, only the data that fits will be written.
  * @param rb Ringbuffer to write to.
  * @param data Data to write
  * @param len Data length
  * @return Amount of data that was actually written.
  */
int Kit_WriteRingBuffer(Kit_RingBuffer *rb, const char* data, int len) {
    int k;
    len = (len > (rb->size - rb->len)) ? (rb->size - rb->len) : len;
    if(rb->len < rb->size) {
        if(len + rb->wpos > rb->size) {
            k = (len + rb->wpos) % rb->size;
            memcpy((rb->data + rb->wpos), data, len - k);
            memcpy(rb->data, data+(len-k), k);
        } else {
            memcpy((rb->data + rb->wpos), data, len);
        }
        rb->len += len;
        rb->wpos += len;
        if(rb->wpos >= rb->size) {
            rb->wpos = rb->wpos % rb->size;
        }
        return len;
    }
    return 0;
}

static void _ReadRingBufferData(const Kit_RingBuffer *rb, char *data, const int len) {
    int k;
    if(len + rb->rpos > rb->size) {
        k = (len + rb->rpos) % rb->size;
        memcpy(data, rb->data + rb->rpos, len - k);
        memcpy(data + (len - k), rb->data, k);
    } else {
        memcpy(data, rb->data + rb->rpos, len);
    }
}

/**
  * Reads data from ringbuffer. If ringbuffer has less data than was requested,
  * only the available data will be read.
  * @param rb Ringbuffer to read from.
  * @param data Buffer to read into.
  * @param len How much data do we want
  * @return Amount of data that was actually read.
  */
int Kit_ReadRingBuffer(Kit_RingBuffer *rb, char *data, int len) {
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        _ReadRingBufferData(rb, data, len);
        rb->len -= len;
        rb->rpos += len;
        if(rb->rpos >= rb->size) {
            rb->rpos = rb->rpos % rb->size;
        }
        return len;
    }
    return 0;
}

/**
  * Peeks into the given ringbuffer. Technically same as rb_read, but does not advance
  * the internal position pointer. In other words, you may peek as many times as you wish,
  * and will always get the same data.
  * @param rb Ringbuffer to peek into.
  * @param data Buffer to read into
  * @param len How much data do we need
  * @return Amount of data actually read
  */
int Kit_PeekRingBuffer(const Kit_RingBuffer *rb, char *data, int len) {
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        _ReadRingBufferData(rb, data, len);
        return len;
    }
    return 0;
}

/**
  * Advances the internal position counter by given amount. Note that counter can only be
  * advanced by the amount of unreadable data in ringbuffer.
  * @param rb Ringbuffer to handle
  * @param len How much should the position counter be increased
  * @return How much the position counter was actually increased.
  */
int Kit_AdvanceRingBuffer(Kit_RingBuffer *rb, int len) {
    len = (len > rb->len) ? rb->len : len;
    if(rb->len > 0) {
        rb->len -= len;
        rb->rpos += len;
        if(rb->rpos >= rb->size) {
            rb->rpos = rb->rpos % rb->size;
        }
        return len;
    }
    return 0;
}

/**
  * Returns the current length of the Ringbuffer. In other words, how much data
  * the ringbuffer contains
  * @param rb Ringbuffer to handle
  * @return Data in ringbuffer (in bytes).
  */
int Kit_GetRingBufferLength(const Kit_RingBuffer *rb) {
    return rb->len;
}

/**
  * Returns the size of the ringbuffer. In other words, the maximum amount of data
  * the ringbuffer can hold.
  * @param rb Ringbuffer to handle
  * @return Size of the ringbuffer
  */
int Kit_GetRingBufferSize(const Kit_RingBuffer *rb) {
    return rb->size;
}

/**
  * Returns the free size of the ringbuffer.
  * @param rb Ringbuffer to handle
  * @return Free size in the ringbuffer
  */
int Kit_GetRingBufferFree(const Kit_RingBuffer *rb) {
    return rb->size - rb->len;
}
