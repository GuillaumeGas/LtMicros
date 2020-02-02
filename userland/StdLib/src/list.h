#pragma once

#include "status.h"

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
    typedef Status(*EnumerateFunPtr)(void*, void*);

    extern "C" List * ListCreate();
    void ListDestroy(List * list);
    void ListDestroyEx(List * list, CleanFunPtr cleaner);
    void ListPush(List * list, void * data);
    void * ListGet(List * list, unsigned int index);
    void * ListTop(List * list);
    void * ListPop(List ** list);
    Status ListEnumerate(List * list, EnumerateFunPtr callback, void * Context);
    bool ListIsEmpty(List* list);

}
/// @}