#include "pch.h"
#include "Stack.h"

//Will only be called when allocating the whole stack memory.
Stack::Stack(unsigned long long stackSize)
{
	//Allocate the stack itself.
	m_pData = DBG_NEW std::byte[stackSize];
	m_stackSize = stackSize;
	m_currentSize = 0;
}

Stack::~Stack()
{
	delete[] m_pData;
	m_pData = nullptr;
}