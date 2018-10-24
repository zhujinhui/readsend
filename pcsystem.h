#ifndef __PRODUCER_AND_CONSUMER_SYSTEM_H__
#define __PRODUCER_AND_CONSUMER_SYSTEM_H__

#include <vector>

namespace P_C_SYS {

using namespace std;

class PCItem
{
public:
    virtual ~PCItem();
};

typedef vector<PCItem *> ItemVector;

class PCStore
{
private:
    int m_nHead;
    int m_nRear;
    int m_nSize;

    ItemVector m_Items;

public:
    static const int TYP_4PRO = 0;
    static const int TYP_4CON = 1;


public:
    PCStore();
    virtual ~PCStore();

    bool AddItem(PCItem * pItem);           //TODO: how can I save PCItem sub class instance into store, and call sub class implementation?
    PCItem * Checkout(const int type);
    PCItem * Checkin(const int type);
};

class PCRunner
{
private:
    bool m_bRunning;

public:
    PCRunner();
    virtual ~PCRunner();

    bool StartRun();
    void StopRun();
    static void * RunnerEntry(void * pCtx);     // while(1): check stop, call process
};

class PCProducer : public PCRunner
{
public:
    void Process(PCItem * pItem);           // call produce()
    virtual void Produce(PCItem * pItem);
};

class PCConsumer : public PCRunner
{
public:
    void Process(PCItem * pItem);           // call consume()
    virtual void Consume(PCItem * pItem);
};

typedef vector<PCProducer *> ProducerVector;
typedef vector<PCConsumer *> ConsumerVector;

class PCSystem
{
private:
    PCStore m_Store;
    ProducerVector m_Producers;
    ConsumerVector m_Consumers;

public:
    bool AddItem(PCItem * pItem);       // item will be deleted by PCSystem

    // runner will be deleted by PCSystem
    bool AddProducer(PCProducer* producer);     // will be runned after starting
    bool AddConsumer(PCConsumer* consumer);

    bool StartSystem();     // after started, the runner added to system will be runned immediately
};

}

#endif

