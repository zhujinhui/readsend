#include <iostream>
#include <sys/prctl.h>
#include "readsend.h"

#define ENTER_CRITICAL_SECTION()
#define LEAVE_CRITICAL_SECTION()

BufferObj::BufferObj()
{
    m_pBuffer = NULL;
    m_bufLen = 0;
    m_dataLen = 0;
}

LoopBuffers::LoopBuffers(int bufLen, int size)
{
    m_first = true;
    m_size = size;

    for (int i=0; i<m_size; i++)
    {
        BufferObj obj;
        obj.m_pBuffer = malloc(bufLen);
        if (obj.m_pBuffer != NULL)
        {
            obj.m_bufLen = bufLen;
            m_BufferVector.push_back(obj);
            cout << "LoopBuffers malloc " << bufLen/1024 << "KB" << endl;
        }
    }

    m_head = 0;
    m_rear = 0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP);
    pthread_mutex_init(&m_mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    m_sema = MPCreateSema(0);
}

LoopBuffers::~LoopBuffers()
{
    for (int i=0; i<m_size; i++)
    {
        BufferObj obj = m_BufferVector.at(i);
        if (obj.m_pBuffer) free(obj.m_pBuffer);
    }
    pthread_mutex_destroy(&m_mutex);
    MPDestroySema(m_sema);
}

void LoopBuffers::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void LoopBuffers::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

bool LoopBuffers::checkout(BufferObj &obj, bool read)
{
    lock();

    if (read)
    {
        // produce request
        if (m_first == false)
        {
            if (m_head == m_rear)
            {
                // full
                //cout << "[RS][READ] LoopBuffer Full rear:" << m_rear << " head:" << m_head << endl;
                unlock();
                return false;
            }
        }
        else
        {
            m_first = false;
        }
        cout << "[RS][READ] checked out rear:" << m_rear << " head:" << m_head << endl;
        obj = m_BufferVector.at(m_rear);
        unlock();
        return true;
    }
    else
    {
        // consume request
        if (m_head == m_rear)
        {
            // empty
            //cout << "[RS][SEND] LoopBuffer Empty rear:" << m_rear << " head:" << m_head << endl;
            unlock();
            return false;
        }
        cout << "[RS][SEND] checked out rear:" << m_rear << " head:" << m_head << endl;
        obj = m_BufferVector.at(m_head);
        unlock();
        return true;
    }
}

void LoopBuffers::checkin(BufferObj obj, bool read)
{
    lock();

    for (int i=0; i<m_size; i++)
    {
        BufferObj tmp = m_BufferVector.at(i);
        if (obj.m_pBuffer == tmp.m_pBuffer)
        {
            if (read)
            {
                // produce finish
                m_rear = (m_rear+1)%m_size;
                cout << "[RS][READ] checked in rear:" << m_rear << " head:" << m_head << endl;
                break;
            }
            else
            {
                // consume finish
                m_head = (m_head+1)%m_size;
                cout << "[RS][SEND] checked in rear:" << m_rear << " head:" << m_head << endl;
                break;
            }
        }
    }

    unlock();
    wakeup();
}

bool LoopBuffers::wait(int ms)
{
    //cout << "[RS] wait -->" << endl;
    bool ok = MPWaitSema(m_sema, ms);
    //cout << "[RS] wait <-- " << ok << endl;
    return ok;
}

void LoopBuffers::wakeup()
{
    //cout << "[RS] wakeup -->" << endl;
    MPSignalSema(m_sema);
    //cout << "[RS] wait <--" << endl;
}

const int ReadThread::__stop = 0;
const int ReadThread::__start = 1;

void * ReadThreadProc(void * pCtx)
{
    if (pCtx == NULL) return NULL;

    ReadThread * pThread = static_cast<ReadThread *>(pCtx);
    prctl(PR_SET_NAME, pThread->m_strName.c_str());

    while (pThread->m_status != ReadThread::__stop)
    {
        pThread->Process();
    }

    return NULL;
}

ReadThread::ReadThread(const char * pszName, LoopBuffers & loopBuffers) : m_LoopBuffers(loopBuffers)
{
    m_strName = pszName;
    m_status = __stop;
}

int ReadThread::StartThread()
{
    pthread_attr_t      attr;
    struct sched_param  param;

    pthread_attr_init(&attr);
    pthread_attr_getschedparam(&attr, &param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    param.sched_priority = sched_get_priority_max(SCHED_OTHER);
    pthread_attr_setschedparam(&attr, &param);

    m_status = __start;
    int ok = pthread_create(&m_threadId, &attr, ReadThreadProc, this);

    if (ok != 0) m_status = __stop;
    return ok;
}

void ReadThread::StopThread()
{
    void * ret = NULL;
    m_status = ReadThread::__stop;
    pthread_join(m_threadId, &ret);     //TODO: if thread waiting for checkout, we can't join it.
}

void ReadThread::Process()
{
    BufferObj obj;
    bool have = m_LoopBuffers.checkout(obj);

    if (have == false)
    {
        have = m_LoopBuffers.wait(30);    // 1.5 X time of sending one buffer
        if (have == false)
            return;         // no buffer for now, return to thread loop, which give a chance to exit thread.
        else
            m_LoopBuffers.checkout(obj);
    }

    Read(obj.m_pBuffer, obj.m_bufLen);
    m_LoopBuffers.checkin(obj);
}

TestReadThread::TestReadThread(const char *pszName, LoopBuffers &loopBuffers) : ReadThread(pszName, loopBuffers)
{
}

void MPSleep(int timeMs)
{
    if (timeMs >= 1000)
    {
        sleep(timeMs / 1000);
    }
    usleep((timeMs % 1000) * 1000);
}

void TestReadThread::Read(void *pBuffer, size_t iBufLen)
{
    MPSleep(5);
    //cout << "TsetReadThread::Read" << endl;
}
