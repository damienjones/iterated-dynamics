#if !defined(JIIM_H)
#define JIIM_H

#include "cmplx.h"

extern InitializedComplexD	g_save_c;

extern void Jiim(bool which);
extern ComplexL PopLong();
extern ComplexD PopFloat();
extern ComplexL DeQueueLong();
extern ComplexD DeQueueFloat();
extern ComplexL ComplexSqrtLong(long,  long);
extern bool Init_Queue(unsigned long request);
extern void   Free_Queue();
extern int    QueueEmpty();
extern int    QueueFull();
extern int    QueueFullAlmost();
extern int    PushLong(long,  long);
extern int    PushFloat(float,  float);
extern int    EnQueueLong(long,  long);
extern int    EnQueueFloat(float,  float);

#endif