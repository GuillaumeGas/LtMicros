#include "cppsupport.hpp"

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("KERNEL", LOG_LEVEL, format, ##__VA_ARGS__)

extern "C" void __cxa_pure_virtual()
{
    KLOG(LOG_ERROR, "Unexpected __cxa_pure_virtual call");
}