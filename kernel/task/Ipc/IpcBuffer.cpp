#include "IpcBuffer.hpp"

#include <kernel/Logger.hpp>

#define KLOG(LOG_LEVEL, format, ...) KLOGGER("IPCBUFFER", LOG_LEVEL, format, ##__VA_ARGS__)
#ifdef DEBUG_DEBUGGER
#define DKLOG(LOG_LEVEL, format, ...) KLOGGER("IPCBUFFER", LOG_LEVEL, format, ##__VA_ARGS__)
#else
#define DKLOG(LOG_LEVEL, format, ...)
#endif

KeStatus IpcBuffer::AddBytes(const char* message, const unsigned int size)
{
    KeStatus status = STATUS_FAILURE;

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

    /*
        - Ecrire ce qu'on peut sur le buffer courant ET d�placer le curseur d'�criture
        - Si on a pas pu tout �crire,
            - allouer une nouvelle page, et ajouter la courante dans la liste de pages. La nouvelle page devient la courante.
            - refaire cette op�ration autant de fois qu'il le faudra
    */

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus IpcBuffer::ReadBytes(char* const buffer, const unsigned int size, unsigned int* const bytesRead)
{
    KeStatus status = STATUS_FAILURE;

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

    /*
        - Essayer de lire les size octets sur la page courante, et mettre � jour le curseur de lecture.
        - Si on est arriv� en haut de la page ET qu'on a pas assez lu d'octets
           - on passe � la page suivante si il y en une, et on fait �a jusqu'� avoir lu les size octets
           - si il n'y a pas assez d'octets � lire, on met � jour bytesRead avec les octets lu

           Si on a pu tout lire, on met *bytesRead = size
    */

    status = STATUS_SUCCESS;

clean:
    return status;
}