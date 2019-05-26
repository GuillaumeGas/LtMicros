#define __PIC_DRIVER__
#include "Pic.hpp"

#include <kernel/drivers/proc_io.hpp>

/// @addgroup DriversGroup
/// @{

#define DEFAULT_ICW1_VALUE 0x10
#define DEFAULT_ICW4_VALUE 0x01

#define INIT_MASTER_PORT 0x20
#define INIT_SLAVE_PORT  0xA0

#define MASTER_PORT 0x21
#define SLAVE_PORT 0xA1

void PicDriver::Init()
{
    SetIcw1(ICW1_WITH_ICW4);
    // master, offset in IDT : 32
    // slave, 96 (even if it's not used right now...)
    SetIcw2(0x20, 0x70);
    // SetIcw3(0x04, 0x02);
    // SetIcw4(PIC_CTRL_DEFAULT);
    SetOcw1(IRQ_SYSTEM_TIMER /*| IRQ_KEYBOARD | IRQ_SERIAL_1*/, IRQ_NONE);

    Reload();

    isInitialized = true;
}

void PicDriver::SetIcw1(Icw1Mask mask)
{
    icw1 = DEFAULT_ICW1_VALUE;
    icw1 |= mask;
}

void PicDriver::SetIcw2(u8 masterIntVecBaseAddr, u8 slaveIntVecBaseAddr)
{
    icw2Master = masterIntVecBaseAddr;
    icw2Slave = slaveIntVecBaseAddr;
}

void PicDriver::SetIcw3(u8 master, u8 slave)
{
    icw3Master = master;
    icw3Slave = slave;
}

void PicDriver::SetIcw4(ControllerModeMask mask)
{
    icw4 = DEFAULT_ICW4_VALUE;
    icw4 |= mask;
}

void PicDriver::SetOcw1(MasterIrqMask master, SlaveIrqMask slave)
{
    ocw1Master = !master;
    ocw1Slave = !slave;
}

void PicDriver::Reload()
{
    outb(INIT_MASTER_PORT, icw1);
    outb(INIT_SLAVE_PORT, icw1);
    
    outbp(MASTER_PORT, icw2Master);
    outbp(SLAVE_PORT, icw2Slave);

    outb(MASTER_PORT, ocw1Master);
    outb(SLAVE_PORT, ocw1Slave);
}

/// @}