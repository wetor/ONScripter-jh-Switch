#ifndef KITRINGBUFFER_H
#define KITRINGBUFFER_H

#include "kitchensink/kitconfig.h"

typedef struct Kit_RingBuffer {
    int size;
    int len;
    int wpos;
    int rpos;
    char* data;
} Kit_RingBuffer;

KIT_LOCAL Kit_RingBuffer* Kit_CreateRingBuffer(unsigned int size);
KIT_LOCAL void Kit_DestroyRingBuffer(Kit_RingBuffer* rb);
KIT_LOCAL int Kit_WriteRingBuffer(Kit_RingBuffer *rb, const char* data, int len);
KIT_LOCAL int Kit_ReadRingBuffer(Kit_RingBuffer *rb, char* data, int len);
KIT_LOCAL int Kit_PeekRingBuffer(const Kit_RingBuffer *rb, char* data, int len);
KIT_LOCAL int Kit_AdvanceRingBuffer(Kit_RingBuffer *rb, int len);
KIT_LOCAL int Kit_GetRingBufferLength(const Kit_RingBuffer *rb);
KIT_LOCAL int Kit_GetRingBufferSize(const Kit_RingBuffer *rb);
KIT_LOCAL int Kit_GetRingBufferFree(const Kit_RingBuffer *rb);

#endif // KITRINGBUFFER_H
