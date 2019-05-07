#include <stdlib.h>
#include <assert.h>

#include "kitchensink/internal/utils/kitbuffer.h"

Kit_Buffer* Kit_CreateBuffer(unsigned int size, Kit_BufferFreeCallback free_cb) {
    Kit_Buffer *b = calloc(1, sizeof(Kit_Buffer));
    if(b == NULL) {
        return NULL;
    }
    b->size = size;
    b->free_cb = free_cb;
    b->data = calloc(size, sizeof(void*));
    if(b->data == NULL) {
        free(b);
        return NULL;
    }
    return b;
}

unsigned int Kit_GetBufferLength(const Kit_Buffer *buffer) {
    return buffer->write_p - buffer->read_p;
}

unsigned int Kit_GetBufferSize(const Kit_Buffer *buffer) {
    return buffer->size;
}

int Kit_GetBufferBufferedSize(const Kit_Buffer *buffer) {
    float total = Kit_GetBufferSize(buffer);
    float current = Kit_GetBufferLength(buffer);
    return (int) ((current / total) * 100.0f);
}

void Kit_DestroyBuffer(Kit_Buffer *buffer) {
    if(buffer == NULL) return;
    Kit_ClearBuffer(buffer);
    free(buffer->data);
    free(buffer);
}

void Kit_ClearBuffer(Kit_Buffer *buffer) {
    void *data;
    if(buffer->free_cb == NULL) {
        return;
    }
    while((data = Kit_ReadBuffer(buffer)) != NULL) {
        buffer->free_cb(data);
    }
}

void* Kit_ReadBuffer(Kit_Buffer *buffer) {
    assert(buffer != NULL);
    if(buffer->read_p < buffer->write_p) {
        void *out = buffer->data[buffer->read_p % buffer->size];
        buffer->data[buffer->read_p % buffer->size] = NULL;
        buffer->read_p++;
        if(buffer->read_p >= buffer->size) {
            buffer->read_p = buffer->read_p % buffer->size;
            buffer->write_p = buffer->write_p % buffer->size;
        }
        return out;
    }
    return NULL;
}

void* Kit_PeekBuffer(const Kit_Buffer *buffer) {
    assert(buffer != NULL);
    if(buffer->read_p < buffer->write_p) {
        return buffer->data[buffer->read_p % buffer->size];
    }
    return NULL;
}

void Kit_AdvanceBuffer(Kit_Buffer *buffer) {
    assert(buffer != NULL);
    if(buffer->read_p < buffer->write_p) {
        buffer->data[buffer->read_p % buffer->size] = NULL;
        buffer->read_p++;
        if(buffer->read_p >= buffer->size) {
            buffer->read_p = buffer->read_p % buffer->size;
            buffer->write_p = buffer->write_p % buffer->size;
        }
    }
}

void Kit_ForEachItemInBuffer(const Kit_Buffer *buffer, Kit_ForEachItemCallback cb, void *userdata) {
    unsigned int read_p = buffer->read_p;
    unsigned int write_p = buffer->write_p;
    while(read_p < write_p) {
        cb(buffer->data[read_p++ % buffer->size], userdata);
        if(read_p >= buffer->size) {
            read_p = read_p % buffer->size;
            write_p = write_p % buffer->size;
        }
    }
}

int Kit_WriteBuffer(Kit_Buffer *buffer, void *ptr) {
    assert(buffer != NULL);
    assert(ptr != NULL);

    if(!Kit_IsBufferFull(buffer)) {
        buffer->data[buffer->write_p % buffer->size] = ptr;
        buffer->write_p++;
        return 0;
    }
    return 1;
}

int Kit_IsBufferFull(const Kit_Buffer *buffer) {
    int len = buffer->write_p - buffer->read_p;
    int k = (len >= buffer->size);
    return k;
}
