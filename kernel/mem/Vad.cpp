#include "Vad.hpp"

#include <kernel/lib/StdMem.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("VAD", LOG_LEVEL, format, ##__VA_ARGS__)

KeStatus Vad::Create(const void* baseAddress, const unsigned int size, bool free, Vad ** const outVad)
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

    localVad->baseAddress = (void*)baseAddress;
    localVad->size = size;
    localVad->free = free;

    *outVad = localVad;
    localVad = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Allocate(const unsigned int size, Vad** const OutVad)
{
    /*
        TODO : 
         - Ajouter les noeuds VAD 'previous' et 'next'
         - Aller chercher un VAD de libre assez grand
         - Couper si nécessaire celui trouvé, si il est trop grand
         - Pour le moment : allouer directement la mémoire physique
    */

    return STATUS_SUCCESS;
}