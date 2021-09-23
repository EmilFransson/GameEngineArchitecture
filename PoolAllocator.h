#pragma once
#include <stdint.h>
#include <memory>
#include <cstddef>
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
	PoolAllocator(const uint64_t entityCapacity = 1000000u);
	~PoolAllocator();
	template<typename... Arguments>
	T* New(Arguments&&... args);
	void Delete(T* pData);
	[[nodiscard]] const uint64_t GetUsage() const noexcept;
	[[nodiscard]] const uint64_t GetCapacity() const noexcept;
	[[nodiscard]] const uint64_t GetEntityUsage() const noexcept;
	[[nodiscard]] const uint64_t GetEntityCapacity() const noexcept;

private:
	PoolChunk<T>* m_pMemoryPool;
	PoolChunk<T>* m_pHead;
	uint64_t m_MaxEntities;
	uint64_t m_BytesCapacity;
	uint64_t m_UsedBytes = 0u;
	uint64_t m_NrOfEntities = 0u;
};

template<class T>
PoolAllocator<T>::PoolAllocator(const uint64_t entityCapacity)
	: m_MaxEntities{ entityCapacity }, m_BytesCapacity{ sizeof(T) * m_MaxEntities }
{
	m_pMemoryPool = new PoolChunk<T>[m_MaxEntities];
	m_pHead = m_pMemoryPool;

	for (uint32_t i{ 0u }; i < m_MaxEntities - 1; i++)
	{
		m_pMemoryPool[i].nextPoolChunk = std::addressof(m_pMemoryPool[i + 1]);
	}
	m_pMemoryPool[m_MaxEntities - 1].nextPoolChunk = nullptr;
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
		return nullptr;

	PoolChunk<T>* pPoolChunk = m_pHead;
	m_pHead = m_pHead->nextPoolChunk;

	m_UsedBytes += sizeof(T);
	m_NrOfEntities++;

	return new(std::addressof(pPoolChunk->data))T(std::forward<Arguments>(args)...);
}

template<class T>
void PoolAllocator<T>::Delete(T* pData)
{
	m_UsedBytes -= sizeof(T);
	pData->~T();
	PoolChunk<T>* poolChunk = reinterpret_cast<PoolChunk<T>*>(pData);
	poolChunk->nextPoolChunk = m_pHead;
	m_pHead = poolChunk;
	m_NrOfEntities--;
}

template<class T>
const uint64_t PoolAllocator<T>::GetUsage() const noexcept
{
	return m_UsedBytes;
}

template<class T>
const uint64_t PoolAllocator<T>::GetCapacity() const noexcept
{
	return m_BytesCapacity;
}

template<class T>
inline const uint64_t PoolAllocator<T>::GetEntityUsage() const noexcept
{
	return m_NrOfEntities;
}

template<class T>
inline const uint64_t PoolAllocator<T>::GetEntityCapacity() const noexcept
{
	return m_MaxEntities;
}
