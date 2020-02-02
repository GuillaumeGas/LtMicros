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

KeStatus Vad::Allocate(const unsigned int size, const PageDirectory * pageDirectory, const bool reservePhysicalPages, Vad** const outVad)
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
        KLOG(LOG_ERROR, "LookForFreeVadOfMinimumSize() failed with code %t", status);
        goto clean;
    }

    if ((freeVad->size - size) > VAD_MINIMUM_DELTA)
    {
        status = freeVad->Split(size);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "Split() failed with code %t", status);
            goto clean;
        }
    }

    status = freeVad->ReservePages(pageDirectory, reservePhysicalPages);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "ReservePages() failed with code %t", status);
        goto clean;
    }

    *outVad = freeVad;
    freeVad = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::AllocateAtAddress(void * const address, const unsigned int size, const PageDirectory * pageDirectory, const bool reservePhysicalPages, Vad** const outVad)
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

    status = LookForFreeVadAtAddressOfMinimumSize(address, size, &freeVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "LookForFreeVadAtAddressOfMinimumSize() failed with code %t", status);
        goto clean;
    }

    // If the asked address is in the middle of the free vad,
    //  or if the vad is too large, we split it
    if (freeVad->baseAddress != address 
        || (freeVad->baseAddress == address && ((freeVad->size - size) > VAD_MINIMUM_DELTA)))
    {
        status = freeVad->SplitAtAddress(address, size);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "SplitAtAddress() failed with code %t", status);
            goto clean;
        }

        if (freeVad->baseAddress != address)
            freeVad = freeVad->next;
    }

    status = freeVad->ReservePages(pageDirectory, reservePhysicalPages);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "ReservePages() failed with code %t", status);
        goto clean;
    }

    *outVad = freeVad;
    freeVad = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::LookForVadFromAddress(void* const address, Vad** const outVad)
{
    KeStatus status = STATUS_FAILURE;
    Vad* vad = nullptr;

    if (address == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid address parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (outVad == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outVad parameter");
        return STATUS_NULL_PARAMETER;
    }

    vad = this;
    do
    {
        if (address >= vad->baseAddress && address < vad->limitAddress)
            break;

        vad = vad->next;

    } while (vad != nullptr);

    if (vad == nullptr)
    {
        status = STATUS_NOT_FOUND;
        goto clean;
    }

    *outVad = vad;
    vad = nullptr;

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

    freeVad = currentVad;

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

    currentVad = this;
    do
    {
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

    freeVad = currentVad;

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
        KLOG(LOG_ERROR, "Create() failed with code %t", status);
        goto clean;
    }

    newVad->previous = this;

    if (newVad->next != nullptr)
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

    // The asked address is in the middle of the free block
    if (this->baseAddress != address)
    {
        Vad * newVad = nullptr;

        // We create the second block starting at the asked address
        unsigned int secondBlockSize = (unsigned int)this->limitAddress - (unsigned int)address;

        // If the second block size that is going to be created won't be large enough
        //  SplitAtAddress() shouldn't have been called
        if (secondBlockSize < size)
        {
            KLOG(LOG_ERROR, "secondBlockSize < asdked size");
            status = STATUS_FAILURE;
            goto clean;
        }

        status = Vad::Create(address, secondBlockSize, true, &newVad);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "Create() failed with code %t", status);
            goto clean;
        }

        newVad->previous = this;
        newVad->next = this->next;

        if (newVad->next != nullptr)
            newVad->next->previous = newVad;

        // We update the first block size and limit address
        this->limitAddress -= secondBlockSize;
        this->size -= secondBlockSize;
        this->next = newVad;

        // If the second block is too big for the asked size, we split it
        if ((newVad->size - size) > VAD_MINIMUM_DELTA)
        {
            status = newVad->Split(size);
            if (FAILED(status))
            {
                KLOG(LOG_ERROR, "Split() failed with code %t", status);
                goto clean;
            }
        }
   }
    else
    {
        // Since the asked address is the current cad base address, we can do a normal split
        status = this->Split(size);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "Split() failed with code %t", status);
            goto clean;
        }
    }

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Vad::ReservePages(const PageDirectory * pageDirectory, const bool reservePhysicalPages)
{
    u8 * vAddr = this->baseAddress;
    PageDirectoryEntry * currentPd = gVmm.GetCurrentPageDirectory();

    gVmm.SetCurrentPageDirectory(pageDirectory->pdEntry);

    while (vAddr < this->limitAddress)
    {
        if (reservePhysicalPages)
        {
            u8* pAddr = (u8*)gPmm.GetFreePage();
            if (pAddr == nullptr)
            {
                KLOG(LOG_ERROR, "Pmm::GetFreePage() returned null");
                return STATUS_PHYSICAL_MEMORY_FULL;
            }

            gVmm.AddPageToPageDirectory((u32)vAddr, (u32)pAddr, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, *pageDirectory);
        }
        else
        {
            // The virtual address doesn't point to any physical address
            // The page is set as not present in memory, so if we try to access it, a page fault will occured and a physical page will be reserved
            gVmm.AddPageToPageDirectory((u32)vAddr, 0, PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, *pageDirectory);
        }

        vAddr = (u8 *)((unsigned int)vAddr + (unsigned int)PAGE_SIZE);
    }

    gVmm.SetCurrentPageDirectory(currentPd);

    this->free = false;

    return STATUS_SUCCESS;
}

void Vad::PrintVad()
{
    Vad * current = this;
    int i = 0;

    while (current != nullptr)
    {
        KLOG(LOG_DEBUG, "vad %d [%x - %x] (%d bytes)", i++, current->baseAddress, current->limitAddress, current->size);
        current = current->next;
    }
}