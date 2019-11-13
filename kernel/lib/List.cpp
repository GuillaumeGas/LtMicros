#include "List.hpp"

#include <kernel/lib/StdLib.hpp>
#include <kernel/lib/StdIo.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("LIB", LOG_LEVEL, format, ##__VA_ARGS__)

extern "C"
{

    extern "C" List * ListCreate()
    {
        ListElem * list = (ListElem *)HeapAlloc(sizeof(ListElem));

        if (list == nullptr)
        {
            KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(ListElem));
            return nullptr;
        }

        list->prev = nullptr;
        list->next = nullptr;
        list->data = nullptr;

        return list;
    }

    void ListDestroy(List * list)
    {
        ListDestroyEx(list, nullptr);
    }

    void ListDestroyEx(List * list, CleanFunPtr cleaner)
    {
        if (list == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid list parameter");
            return;
        }

        ListElem * elem = list;
        ListElem * next = list->next;

        while (elem != nullptr)
        {
            if (cleaner != nullptr && elem->data != nullptr)
                cleaner(elem->data);
            HeapFree(elem);
            elem = next;

            if (next != nullptr)
                next = next->next;
        }
    }

    void ListPush(List * list, void * data)
    {
        if (list == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid list parameter");
            return;
        }

        if (list->data == nullptr)
        {
            list->data = data;
        }
        else
        {
            ListElem * elem = list;

            while (elem->next != nullptr)
            {
                elem = elem->next;
            }

            elem->next = (ListElem *)HeapAlloc(sizeof(ListElem));

            if (elem->next == nullptr)
            {
                KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(ListElem));
                return;
            }

            elem->next->prev = elem;
            elem = elem->next;
            elem->data = data;
            elem->next = nullptr;
        }
    }

    void * ListGet(List * list, unsigned int index)
    {
        if (list == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid list parameter");
            return nullptr;
        }

        while (index > 0)
        {
            list = list->next;
            index--;

            if (list->next == nullptr)
                break;
        }

        if (index == 0)
            return list->data;
        return nullptr;
    }

    void * ListTop(List * list)
    {
        if (list == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid list parameter");
            return nullptr;
        }

        return list->data;
    }

    void * ListPop(List ** list)
    {
        if (list == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid list parameter");
            return nullptr;
        }

        if ((*list) == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid list parameter");
            return nullptr;
        }

        void * data = (*list)->data;
        ListElem * next = (*list)->next;
        HeapFree(*list);
        *list = (List *)next;
        return data;
    }

    void ListEnumerate(List * list, EnumerateFunPtr callback, void * Context)
    {
        if (list == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid list parameter");
            return;
        }
        if (callback == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid callback parameter");
            return;
        }

        if (ListIsEmpty(list))
            return;

        ListElem * elem = list;
        ListElem * next = list->next;

        while (elem != nullptr)
        {
            callback(elem->data, Context);
            elem = next;

            if (next != nullptr)
                next = next->next;
        }
    }

    bool ListIsEmpty(List* list)
    {
        if (list == nullptr)
        {
            KLOG(LOG_ERROR, "Invalid list parameter");
            return true;
        }

        if (list->data == nullptr && list->next == nullptr)
            return true;
        return false;
    }
}