#include "Vad.hpp"

#include <kernel/lib/StdMem.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/arch/x86/Pmm.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("VAD", LOG_LEVEL, format, ##__VA_ARGS__)

#define VAD_MINIMUM_DELTA PAGE_SIZE

KeStatus Vad::Create(void * const baseAddress, const unsigned int size, bool free, Vad ** const outVad)
{
    KeStatus status = STATUS_FAILURE;
    Vad* localVad = nullptr;

    if (baseAddress == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid baseAddress parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (outVad == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outVad parameter");
        return STATUS_NULL_PARAMETER;
    }

    localVad = (Vad*)HeapAlloc(sizeof(Vad));
    if (localVad == nullptr)
    {
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    localVad->baseAddress = (u8*)baseAddress;
    localVad->limitAddress = (u8*)baseAddress + size;
    localVad->size = size;
    localVad->free = free;
    localVad->previous = nullptr;
    localVad->next = nullptr;

    *outVad = localVad;
    localVad = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::Allocate(const unsigned int size, const PageDirectory * pageDirectory, Vad** const outVad)
{
    KeStatus status = STATUS_FAILURE;
    Vad * freeVad = nullptr;

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (pageDirectory == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid pageDirectory parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (outVad == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outVad parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = LookForFreeVadOfMinimumSize(size, &freeVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "LookForFreeVadOfMinimumSize() failed with code %d", status);
        goto clean;
    }

    if ((freeVad->size - size) > VAD_MINIMUM_DELTA)
    {
        status = freeVad->Split(size);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "Split() failed with code %d", status);
            goto clean;
        }
    }

    status = freeVad->ReserveAndAllocateMemory(pageDirectory);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "ReserveAndAllocateMemory() failed with code %d", status);
        goto clean;
    }

    *outVad = freeVad;
    freeVad = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::AllocateAtAddress(void * const address, const unsigned int size, const PageDirectory * pageDirectory, Vad** const outVad)
{
    KeStatus status = STATUS_FAILURE;
    Vad * freeVad = nullptr;

    if (address == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid address parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (pageDirectory == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid pageDirectory parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (outVad == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outVad parameter");
        return STATUS_NULL_PARAMETER;
    }

    KLOG(LOG_DEBUG, "test1");
    status = LookForFreeVadAtAddressOfMinimumSize(address, size, &freeVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "LookForFreeVadAtAddressOfMinimumSize() failed with code %d", status);
        goto clean;
    }

    if ((freeVad->size - size) > VAD_MINIMUM_DELTA)
    {
        KLOG(LOG_DEBUG, "test2");
        status = freeVad->SplitAtAddress(address, size);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "SplitAtAddress() failed with code %d", status);
            goto clean;
        }

        // Since the vad has been split, we want the next one
        freeVad = freeVad->next;
    }

    KLOG(LOG_DEBUG, "test3");
    status = freeVad->ReserveAndAllocateMemory(pageDirectory);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "ReserveAndAllocateMemory() failed with code %d", status);
        goto clean;
    }
    KLOG(LOG_DEBUG, "test4");

    *outVad = freeVad;
    freeVad = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::LookForFreeVadOfMinimumSize(const unsigned int size, Vad ** const outVad)
{
    KeStatus status = STATUS_FAILURE;
    Vad * freeVad = nullptr;
    Vad * currentVad = nullptr;

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (outVad == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outVad parameter");
        return STATUS_NULL_PARAMETER;
    }

    currentVad = this;

    while (currentVad->size < size || !currentVad->free)
    {
        currentVad = currentVad->next;

        if (currentVad == nullptr)
            break;
    }

    if (currentVad == nullptr)
    {
        KLOG(LOG_WARNING, "No available VAD was found (size : %d)", size);
        status = STATUS_NOT_FOUND;
        goto clean;
    }

    *outVad = freeVad;
    freeVad = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::LookForFreeVadAtAddressOfMinimumSize(void * const address, const unsigned int size, Vad ** const outVad)
{
    KeStatus status = STATUS_FAILURE;
    Vad * freeVad = nullptr;
    Vad * currentVad = nullptr;

    if (address == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid address parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (outVad == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outVad parameter");
        return STATUS_NULL_PARAMETER;
    }

    KLOG(LOG_DEBUG, "Require %x (%d bytes)", address, size);

    currentVad = this;
    do
    {
        KLOG(LOG_DEBUG, "%x - %x (%d) [%c]", currentVad->baseAddress, currentVad->limitAddress, currentVad->size, currentVad->free ? 'f' : 'r');

        if (address >= currentVad->baseAddress && address < currentVad->limitAddress)
        {
            unsigned int innerSize = 0;

            // We don't want the vad size, but the size between the wanted address and the limit
            innerSize = (unsigned int)currentVad->limitAddress - (unsigned int)address;

            if (!currentVad->free || innerSize < size)
            {
                KLOG(LOG_DEBUG, "Address %x is in reserved vad or the vad size doesn't match (%d/%d)", address, size, currentVad->size);
                status = STATUS_NOT_FOUND;
                goto clean;
            }

            break;
        }

        currentVad = currentVad->next;

    } while (currentVad != nullptr && (currentVad->size < size || !currentVad->free));

    if (currentVad == nullptr)
    {
        KLOG(LOG_WARNING, "No available VAD was found (size : %d)", size);
        status = STATUS_NOT_FOUND;
        goto clean;
    }

    *outVad = freeVad;
    freeVad = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

/* TODO : find a way to implemet mod */
static unsigned int _local_mod(unsigned int a, unsigned int b)
{
    while (a > b)
        a -= b;
    return (a > 0) ? 1 : 0;
}

KeStatus Vad::Split(const unsigned int size)
{
    KeStatus status = STATUS_FAILURE;
    Vad * newVad = nullptr;
    unsigned int nbPages = size / PAGE_SIZE;

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (size > this->size)
    {
        KLOG(LOG_ERROR, "Unexpected size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if ((this->size - size) < VAD_MINIMUM_DELTA)
    {
        KLOG(LOG_ERROR, "This vad shouldn't be split");
        return STATUS_INVALID_PARAMETER;
    }

    if (_local_mod(size, PAGE_SIZE) > 0)
    {
        nbPages++;
    }

    status = Vad::Create(this->baseAddress + (nbPages * PAGE_SIZE), this->size - (nbPages * PAGE_SIZE), true, &newVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Create() failed with code %d", status);
        goto clean;
    }

    newVad->previous = this;
    newVad->next = this->next;
    newVad->next->previous = newVad;

    this->limitAddress = this->baseAddress + (nbPages * PAGE_SIZE);
    this->size = (nbPages * PAGE_SIZE);
    this->next = newVad;
    
    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::SplitAtAddress(void * const address, const unsigned int size)
{
    KeStatus status = STATUS_FAILURE;
    Vad * newVad = nullptr;
    unsigned int nbPages = size / PAGE_SIZE;

    if (address == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid address parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (size > this->size)
    {
        KLOG(LOG_ERROR, "Unexpected size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if ((this->size - size) < VAD_MINIMUM_DELTA)
    {
        KLOG(LOG_ERROR, "This vad shouldn't be split");
        return STATUS_INVALID_PARAMETER;
    }

    if (_local_mod(size, PAGE_SIZE) > 0)
    {
        nbPages++;
    }

    status = Vad::Create(address, nbPages * PAGE_SIZE, true, &newVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Create() failed with code %d", status);
        goto clean;
    }

    newVad->previous = this;
    newVad->next = this->next;
    newVad->next->previous = newVad;

    this->size -= (nbPages * PAGE_SIZE);
    this->limitAddress = this->baseAddress + this->size;
    this->next = newVad;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::ReserveAndAllocateMemory(const PageDirectory * pageDirectory)
{
    u8 * vAddr = this->baseAddress;
    PageDirectoryEntry * currentPd = gVmm.GetCurrentPageDirectory();

    gVmm.SetCurrentPageDirectory(pageDirectory->pdEntry);

    while (vAddr < this->limitAddress)
    {
        u8 * pAddr = (u8*)gPmm.GetFreePage();
        if (pAddr == nullptr)
        {
            KLOG(LOG_ERROR, "Pmm::GetFreePage() returned null");
            return STATUS_PHYSICAL_MEMORY_FULL;
        }

        gVmm.AddPageToPageDirectory((u32)vAddr, (u32)pAddr, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, *pageDirectory);

        vAddr = (u8 *)((unsigned int)vAddr + (unsigned int)PAGE_SIZE);
    }

    gVmm.SetCurrentPageDirectory(currentPd);

    this->free = false;

    return STATUS_SUCCESS;
}