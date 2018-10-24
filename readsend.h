#ifndef __READ_AND_SEND_H__
#define __READ_AND_SEND_H__

#include <pthread.h>
#include <string>
#include <vector>

#include "rssem.h"

using namespace std;

class BufferObj
{
public:
    BufferObj();
    void * m_pBuffer;
    size_t m_bufLen;
    size_t m_dataLen;
};

class LoopBuffers
{
private:
    int m_head;
    int m_rear;
    int m_size;
    int m_bufLen;
    bool m_first;
    pthread_mutex_t m_mutex;
    mpSema m_sema;

    vector<BufferObj> m_BufferVector;

public:
    LoopBuffers(int bufLen, int size);
    ~LoopBuffers();

    void lock();     // protect cursor
    void unlock();

    bool wait(int ms);  // if checkout fail, means there is not buffer to use, wait for ms then checkout again to have a try
    void wakeup();

    bool checkout(BufferObj & obj, bool read = true);
    void checkin(BufferObj obj, bool read = true);
};

class ReadThread
{
private:
    pthread_t m_threadId;
    LoopBuffers& m_LoopBuffers;

public:
    static const int __stop;
    static const int __start;

    int m_status;
    string m_strName;

    ReadThread(const char * pszName, LoopBuffers & loopBuffers);
    //~ReadThread();

    int StartThread();
    void StopThread();
    void Process();
    virtual void Read(void * pBuffer, size_t iBufLen) = 0;
};

class TestReadThread : public ReadThread
{
public:
    TestReadThread(const char * pszName, LoopBuffers & loopBuffers);
    void Read(void *pBuffer, size_t iBufLen);
};

void MPSleep(int timeMs);

#endif

