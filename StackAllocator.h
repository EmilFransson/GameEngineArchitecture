#pragma once
#include "pch.h"
#include "Stack.h"
#include <stdint.h>
#include <memory>
#include <cstddef>
#include <iostream>
#include <string>
#include <assert.h>

class StackAllocator
{
    //Protected so that no one else can create StackAllocators.
protected:
    //Constructor is called only by CreateAllocator.
    StackAllocator(unsigned long long);
    //Called when we delete the allocator in the FreeMemory-function.
    ~StackAllocator();

public:
    //Remove the copy and assign.
    StackAllocator(StackAllocator& other) = delete;
    void operator=(const StackAllocator&) = delete;

    //Used to create the allocator if it does not exist.
    static void CreateAllocator(unsigned long long);
    static void FreeAllMemory();
    //Returns the instance of the singleton.
    static StackAllocator* GetInstance();
    
    //Get stack size and current stack size.
    size_t GetStackMaxSize();
    size_t GetStackCurrentSize();
    //Create a new object.
    template<typename T, typename... Arguments>
    T* New(Arguments&&... args);

    //Called at the end of our scope each frame to clear the stack.
    void CleanUp();

    void ToggleEnabled() noexcept;
    const bool IsEnabled() const noexcept;
private:
    bool m_Enabled;
    //Singleton instance.
    static StackAllocator* pInstance;

    //Our stack header.
    static Stack* m_pMemoryStack;
    //Walks across our stack to assign addresses to created objects and their headers.
    static std::byte* m_pByteWalker;
    //Pointer to top object in the stack.
    static ObjectHeader* m_pTop;
};

StackAllocator* StackAllocator::pInstance{ nullptr };
Stack* StackAllocator::m_pMemoryStack{ nullptr };
std::byte* StackAllocator::m_pByteWalker{ nullptr };
ObjectHeader* StackAllocator::m_pTop{ nullptr };

//---------------------------------------------------------------------

void StackAllocator::CreateAllocator(unsigned long long stackSize)
{
    //Only create it if it does not exist.
    if (!pInstance)
    {
        pInstance = DBG_NEW StackAllocator(stackSize);
    }
    else
    {
        std::cerr << "Error! You are trying to create another Stack Allocator!" << std::endl;
        assert(false);
    }
}

void StackAllocator::ToggleEnabled() noexcept
{
    m_Enabled = !m_Enabled;
}

const bool StackAllocator::IsEnabled() const noexcept
{
    return m_Enabled;
}

void StackAllocator::FreeAllMemory()
{
    delete pInstance;
    pInstance = nullptr;
}

StackAllocator* StackAllocator::GetInstance()
{
    return pInstance;
}

size_t StackAllocator::GetStackMaxSize()
{
    return m_pMemoryStack->m_stackSize;
}

size_t StackAllocator::GetStackCurrentSize()
{
    return m_pMemoryStack->m_currentSize;
}

StackAllocator::StackAllocator(unsigned long long stackSize)
{
    //Allocate the "header" of the stack.
    m_pMemoryStack = DBG_NEW Stack(stackSize);

    //Set the bytewalker to be the start of the allocated memory.
    m_pByteWalker = m_pMemoryStack->m_pData;

    m_pTop = nullptr;

    m_Enabled = false;
}

StackAllocator::~StackAllocator()
{
    delete m_pMemoryStack;
    m_pMemoryStack = nullptr;
    m_pByteWalker = nullptr;
    m_pTop = nullptr;
}

template<typename T, typename... Arguments>
T* StackAllocator::New(Arguments&&... args)
{
    //Check if we have enough space on the stack for the object.
    //If not, return nullptr and nothing happens.
    if (m_pMemoryStack->m_stackSize < m_pMemoryStack->m_currentSize + sizeof(T) + sizeof(ObjectHeader))
    {
        return nullptr;
    }

    //Get an address for the new object and then move the walker by the size of the object.
    std::byte* pObjectDataChunk = m_pByteWalker;
    m_pByteWalker += sizeof(T);

    //Create the object at the assigned address. Increase the currentsize of the stack.
    T* newObject = new(pObjectDataChunk)T(std::forward<Arguments>(args)...);
    m_pMemoryStack->m_currentSize += sizeof(T);

    //Create a header for the new object at the current position of the walker and then move the walker.
    std::byte* pHeaderDataChunk = m_pByteWalker;
    m_pByteWalker += sizeof(ObjectHeader);

    //"Create" or "fill in" the ObjectHeader "object". At the assigned address. Increase the current size of the stack.
    ObjectHeader* header = new(pHeaderDataChunk)ObjectHeader(
        m_pTop,
        newObject,
        [](const void* x) {static_cast<const T*>(x)->~T(); }
    );
    m_pMemoryStack->m_currentSize += sizeof(ObjectHeader);

    //Set the new top element to the new header.
    m_pTop = header;

    return newObject;
}

void StackAllocator::CleanUp()
{
    //Reset pointer & currentsize & call destructor of all allocated objects.

    //Set the bytewalker to be the start of the allocated memory.
    m_pByteWalker = m_pMemoryStack->m_pData;
    //Set current stacksize to 0.
    m_pMemoryStack->m_currentSize = 0;

    //Until all objects' destructors have been called.
    while (m_pTop)
    {
        //Calls the destructor of the object.
        m_pTop->m_pDestructor(m_pTop->m_pObject);

        //Move the top pointer to the next object in the stack.
        m_pTop = m_pTop->m_pObjectHeaderUnder;

        //Do not need to call destructor of the headers, they do not allocate anything.
        //Simply overwrite them next frame.
        //Same with stack variables inside the objects.
    }
}