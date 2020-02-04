#include "IpcBuffer.hpp"

#include <kernel/Logger.hpp>
#include <kernel/mem/PagePool.hpp>

#define KLOG(LOG_LEVEL, format, ...) KLOGGER("IPCBUFFER", LOG_LEVEL, format, ##__VA_ARGS__)
#ifdef DEBUG_DEBUGGER
#define DKLOG(LOG_LEVEL, format, ...) KLOGGER("IPCBUFFER", LOG_LEVEL, format, ##__VA_ARGS__)
#else
#define DKLOG(LOG_LEVEL, format, ...)
#endif

IpcBuffer::IpcBuffer()
{

}

void IpcBuffer::Init()
{
    currentPageWrite = nullptr;
    currentPageRead = nullptr;

    currentPageWritePtr = nullptr;
    currentPageReadPtr = nullptr;

    currentWriteLimit = nullptr;
    currentReadLimit = nullptr;

    pagesList = ListCreate();
}

KeStatus IpcBuffer::AddBytes(const char* message, const unsigned int size)
{
    KeStatus status = STATUS_FAILURE;
    char * currentPage = nullptr;
    char * localMessage = (char*)message;
    unsigned int remainingBytes = size;

    if (message == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid message parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (this->currentPageWritePtr == nullptr)
    {
        status = AllocateWritePage();
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "AllocateWritePage() failed with code %t", status);
            goto clean;
        }
    }

    do
    {
        while (remainingBytes > 0 && (this->currentPageWritePtr < this->currentWriteLimit))
        {
            *(this->currentPageWritePtr) = *localMessage;

            this->currentPageWritePtr++;
            localMessage++;
            remainingBytes--;

            if (this->currentPageWrite == this->currentPageRead)
                this->currentReadLimit++;
        }
        if (remainingBytes > 0)
        {
            status = AllocateWritePage();
            if (FAILED(status))
            {
                KLOG(LOG_ERROR, "AllocateWritePage() failed with code %t", status);
                goto clean;
            }
        }
    } while (remainingBytes > 0);

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus IpcBuffer::ReadBytes(char* const buffer, const unsigned int size, unsigned int* const bytesRead)
{
    KeStatus status = STATUS_FAILURE;
    unsigned int localSize = size;
    char * localBuffer = buffer;

    if (buffer == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid buffer parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (bytesRead == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid bytesRead parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (this->currentPageReadPtr == nullptr)
    {
        Page * nextPage = (Page*)ListTop(this->pagesList);
        this->currentPageRead = nextPage;
        this->currentPageReadPtr = (char*)nextPage->vAddr;

        if (this->currentPageRead == this->currentPageWrite)
            this->currentReadLimit = this->currentPageWritePtr;
        else
            this->currentReadLimit = (char*)(nextPage->vAddr + PAGE_SIZE);
    }

    do
    {
        while (localSize > 0 && (this->currentPageReadPtr < this->currentReadLimit))
        {
            //KLOG(LOG_DEBUG, "[%c]", *(this->currentPageReadPtr));

            *localBuffer = *(this->currentPageReadPtr);
            localBuffer++;
            this->currentPageReadPtr++;
            localSize--;
        }

        if (localSize > 0)
        {
            // If we are not on the last page, we may remove it from the list and free memory
            if (this->currentPageWrite != this->currentPageRead)
            {
                // removing the older page, on the top of the list
                Page * page = (Page*)ListPop(&this->pagesList);

                // we release the page
                gPagePool.Free(*page);

                // And we take the next one
                page = (Page*)ListTop(this->pagesList);

                this->currentPageReadPtr = (char*)page->vAddr;
                this->currentReadLimit = (char*)(page->vAddr + PAGE_SIZE);
            }
            else
            {
                break;
            }
        }
    } while (localSize > 0);

    *bytesRead = (localSize == 0 ? size : (size - localSize));

    status = STATUS_SUCCESS;

clean:
    return status;
}

Page * IpcBuffer::AllocatePage() const
{
    Page * newPage = nullptr;
    Page p = gPagePool.Allocate();

    if (p.vAddr == 0)
    {
        return nullptr;
    }

    newPage = (Page*)HeapAlloc(sizeof(Page));
    if (newPage == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Page));
        return nullptr;
    }

    newPage->pAddr = p.pAddr;
    newPage->vAddr = p.vAddr;

    return newPage;
}

KeStatus IpcBuffer::AllocateWritePage()
{
    Page * newPage = AllocatePage();
    if (newPage == nullptr)
    {
        KLOG(LOG_ERROR, "AllocatePage() failed");
        return STATUS_ALLOC_FAILED;
    }

    this->currentPageWrite = newPage;
    this->currentPageWritePtr = (char*)newPage->vAddr;
    this->currentWriteLimit = (char *)(newPage->vAddr + PAGE_SIZE);

    ListPush(this->pagesList, newPage);

    return STATUS_SUCCESS;
}