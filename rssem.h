#ifndef RSSEM_H
#define RSSEM_H

#include <pthread.h>

#define MP_DWINFINIT            (-1)

typedef void* mpSema;

typedef struct _tagmpSemaObj
{
    pthread_mutex_t   AccessMutex;
    pthread_cond_t    WaitCondition;
    int               nSignalCnt;

} mpSemaObj;

mpSema MPCreateSema(int nInitCnt);
void MPDestroySema(mpSema hSema);
bool MPWaitSema(mpSema hSema, int dwMSec);
void MPSignalSema(mpSema hSema);

#endif // RSSEM_H

