#include "pcsystem.h"

PCItem::~PCItem()
{

}

PCStore::PCStore()
{
    m_nHead = 0;
    m_nRear = 0;
    m_nSize = 0;
}

PCStore::~PCStore()
{

}

bool PCStore::AddItem(PCItem * pItem)
{

}

PCItem * PCStore::Checkout(const int type)
{

}

PCItem * PCStore::Checkin(const int type)
{

}
