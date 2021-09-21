#pragma once

class ObjectHeader
{
public:
    ObjectHeader(ObjectHeader* objectUnder, const void* object, void (*destructor)(const void*))
        : m_pObjectHeaderUnder(objectUnder), m_pObject(object), m_pDestructor(destructor)
    {
        return;
    }
    ~ObjectHeader() = default;

    //Pointer to object below this one in the stack.
    ObjectHeader* m_pObjectHeaderUnder;
    //Pointer to the object that this header is for.
    const void* m_pObject;
    //Pointer to the objects destructor.
    void (*m_pDestructor)(const void*);
private:
};

class Stack
{
public:
    Stack(unsigned long long stackSize);
    ~Stack();

    //Pointer to the actual data.
    std::byte* m_pData;
    //Size of the stack.
    size_t m_stackSize;
    //Current space taken in the stack.
    size_t m_currentSize;
private:
};