#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "rssem.h"

int64_t MPGetMonotonicNs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

int64_t MPGetMonotonicUs()
{
    return MPGetMonotonicNs() / 1000;
}

int64_t MPGetMonotonicMs()
{
    return MPGetMonotonicNs() / 1000000;
}

mpSema MPCreateSema(int nInitCnt)
{
    pthread_condattr_t attr;
    mpSemaObj * pObj = (mpSemaObj *)malloc(sizeof(mpSemaObj));

    if (pObj == NULL)
    {
        return NULL;;
    }

    pObj->nSignalCnt = nInitCnt;

    if (pthread_mutex_init(&pObj->AccessMutex, NULL) != 0)
    {
        goto err_clean;
    }

    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);        // CLOCK_REALTIME will jump with NTP adjust.

    if (pthread_cond_init(&pObj->WaitCondition, &attr) != 0)
    {
        goto err_clean;
    }
    return pObj;

err_clean:
    pthread_mutex_destroy(&pObj->AccessMutex);
    pthread_cond_destroy(&pObj->WaitCondition);
    free(pObj);
    return NULL;
}


void MPDestroySema(mpSema hSema)
{
    mpSemaObj * pObj = (mpSemaObj *)hSema;

    if (pObj != NULL)
    {
        pthread_mutex_destroy(&pObj->AccessMutex);
        pthread_cond_destroy(&pObj->WaitCondition);
        free(pObj);
    }
}

bool MPWaitSema(mpSema hSema, int dwMSec)
{
    int ok = 0;
    bool ret = true;

    struct timespec ts;
    int64_t timeout;
    mpSemaObj * pObj = (mpSemaObj *)hSema;

    if (pObj == NULL) return false;

    pthread_mutex_lock(&pObj->AccessMutex);

    //while(pObj->nSignalCnt == 0)
    {
        if (dwMSec == MP_DWINFINIT)
        {
            ok = pthread_cond_wait(&pObj->WaitCondition, &pObj->AccessMutex);

            if (ok != 0)
            {
                ret = false;
                goto err_clean;
            }
        }
        else
        {
            timeout = MPGetMonotonicMs() + dwMSec;
            ts.tv_sec = timeout / 1000;
            ts.tv_nsec = (timeout % 1000) * 1000000;

            ok = pthread_cond_timedwait(&pObj->WaitCondition, &pObj->AccessMutex, &ts);

            if (ok == ETIMEDOUT)
            {
                ret = false;
                goto err_clean;
            }
            else if (ok != 0)
            {
                ret = false;
                goto err_clean;
            }
        }
    }
    //pObj->nSignalCnt--;

err_clean:
    pthread_mutex_unlock(&pObj->AccessMutex);
    return ret;
}

void MPSignalSema(mpSema hSema)
{
    mpSemaObj * pObj = (mpSemaObj *)hSema;
    if (pObj == NULL) return;

    pthread_mutex_lock(&pObj->AccessMutex);
    //pObj->nSignalCnt++;
    pthread_cond_broadcast(&pObj->WaitCondition);
    pthread_mutex_unlock(&pObj->AccessMutex);
    return;
}
