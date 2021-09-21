#pragma once
#include <stdint.h>
#include <memory>
#include <cstddef>
#define MAX_ENTITIES 1000000
template<typename T>
struct PoolChunk
{
	alignas(alignof(T))
	std::byte data[sizeof(T)];
	PoolChunk<T>* nextPoolChunk;
};

template<class T>
class PoolAllocator
{
public:
	PoolAllocator();
	~PoolAllocator();
	template<typename... Arguments>
	T* New(Arguments&&... args);
	void Delete(T* pData);

private:
	PoolChunk<T>* m_pMemoryPool;
	PoolChunk<T>* m_pHead;
};

template<class T>
PoolAllocator<T>::PoolAllocator()
{
	m_pMemoryPool = new PoolChunk<T>[MAX_ENTITIES];
	m_pHead = m_pMemoryPool;

	for (uint32_t i{ 0u }; i < MAX_ENTITIES - 1; i++)
	{
		m_pMemoryPool[i].nextPoolChunk = std::addressof(m_pMemoryPool[i + 1]);
	}
	m_pMemoryPool[MAX_ENTITIES - 1].nextPoolChunk = nullptr;
}

template<class T>
PoolAllocator<T>::~PoolAllocator()
{
	delete[] m_pMemoryPool;
	m_pMemoryPool = nullptr;
	m_pHead = nullptr;
}

template<class T>
template<typename ...Arguments>
T* PoolAllocator<T>::New(Arguments&&... args)
{
	if (m_pHead == nullptr)
		__debugbreak();

	PoolChunk<T>* pPoolChunk = m_pHead;
	m_pHead = m_pHead->nextPoolChunk;

	return new(std::addressof(pPoolChunk->data))T(std::forward<Arguments>(args)...);
}

template<class T>
void PoolAllocator<T>::Delete(T* pData)
{
	pData->~T();
	PoolChunk<T>* poolChunk = reinterpret_cast<PoolChunk<T>*>(pData);
	poolChunk->nextPoolChunk = m_pHead;
	m_pHead = poolChunk;
}