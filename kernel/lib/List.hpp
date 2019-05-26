#pragma once

/// @defgroup KernelLibGroup Kernel lib group
/// @{

// TODO : new LinkedList template class

extern "C"
{
    struct ListElem
    {
        struct ListElem * prev;
        struct ListElem * next;
        void * data;
    };
    typedef ListElem List;

    typedef void(*CleanFunPtr)(void*);
    typedef void(*EnumerateFunPtr)(void*, void*);

    extern "C" List * ListCreate();
    void ListDestroy(List * list);
    void ListDestroyEx(List * list, CleanFunPtr cleaner);
    void ListPush(List * list, void * data);
    void * ListGet(List * list, unsigned int index);
    void * ListTop(List * list);
    void * ListPop(List ** list);
    void ListEnumerate(List * list, EnumerateFunPtr callback, void * Context);

}
/// @}