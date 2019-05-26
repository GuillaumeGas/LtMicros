#pragma once

/// @file

#include "BaseDriver.hpp"

#include <kernel/lib/Types.hpp>

/// @defgroup DriversGroup Drivers group
/// @{

/// @brief Tells the PIC (Programmable Interrupt Controller) that the current action is completed
#define EOI() asm("mov al, 0x20; out 0x20, al"::);

/// @brief Enum used to initialize icw1
enum Icw1Mask
{
    ICW1_WITH_ICW4         = 0x1,
    ICW1_SINGLE_CONTROLLER = 0x2, // else, cascaded controller
    ICW1_LEVEL_MODE        = 0x8  // else, edge
};

/// @brief Enum used to configurate icw4
enum ControllerModeMask
{
    PIC_CTRL_DEFAULT      = 0x1,
    PIC_CTRL_AEOI         = 0x2,
    PIC_CTRL_MASTER       = 0x4,
    PIC_CTRL_BUFFERED     = 0x8,
    PIC_CTRL_FULLY_NESTED = 0x10,
};

/// @brief Enum used to unmask interrupts on master controller (IRQs 0-7)
enum MasterIrqMask
{
    IRQ_SYSTEM_TIMER = 0x1,
    IRQ_KEYBOARD     = 0x2,
    IRQ_2            = 0x4,
    IRQ_SERIAL_2     = 0x8,
    IRQ_SERIAL_1     = 0x10,
    IRQ_PARALLEL_2   = 0x20,
    IRQ_FLOPPY       = 0x40,
    IRQ_PARALLEL_1   = 0x80
};

/// @brief Enum used to unmask interrupts on slave controller (IRQs 8-15)
enum SlaveIrqMask
{
    IRQ_NONE          = 0x0,
    IRQ_RTC           = 0x1,
    IRQ_ACPI          = 0x2,
    IRQ_10            = 0x4,
    IRQ_USB           = 0x8,
    IRQ_PS_2          = 0x10,
    IRQ_MATH          = 0x20,
    IRQ_PRIMARY_ATA   = 0x40,
    IRQ_SECONDARY_ATA = 0x80
};

///  @brief Programmable Interrupt Controller (PIC) driver (8259A)
///           Composed of icwX members (ICW : Initialization Command Word) and ocwX members (OCW : Operation Control Word)
///         Details regarding hardware interrupts (IRQs for Interrupt ReQuests) list :
///           Master PIC
///             IRQ 0 : System timer
///             IRQ 1 : Keyboard controller
///             IRQ 2 : N/A (cascaded signals from IRQs 8-15)
///             IRQ 3 : Serial port controller (COM2 / shared with COM4 if present)
///             IRQ 4 : Serial port controller (COM1 / shared with COM3 if present)
///             IRQ 5 : Parallel port 2 and 3 or sound card
///             IRQ 6 : Floppy disk controller
///             IRQ 7 : Parallel port 1
///           Salve PIC
///             IRQ 8 : Real-time clock (RTC)
///             IRQ 9 : Advanced Configuration and Power Interface (ACPI) system control
///             IRQ 10 : N/A
///             IRQ 11 : N/A (USB)
///             IRQ 12 : N/A (PS/2)
///             IRQ 13 : Coprocesseur math.
///             IRQ 14 : Primary ATA channel
///             IRQ 15 : Secondary ATA channel
class PicDriver : public BaseDriver
{
public:
    ///@brief Initizalizes the PIC using default LtMicros kernel configuration
    void Init();

    void SetIcw1(Icw1Mask mask);

    void SetIcw2(u8 masterIntVecBaseAddr, u8 slaveIntVecBaseAddr);

    void SetIcw3(u8 master, u8 slave);

    void SetIcw4(ControllerModeMask mask);

    void SetOcw1(MasterIrqMask master, SlaveIrqMask slave);

    /// @brief Configures PIC with the current configuration
    void Reload();

private:
    u8 icw1;

    u8 icw2Master;
    u8 icw2Slave;

    u8 icw3Master;
    u8 icw3Slave;

    u8 icw4;

    u8 ocw1Master;
    u8 ocw1Slave;
};

#ifdef __PIC_DRIVER__
PicDriver gPicDrv;
#else
extern PicDriver gPicDrv;
#endif

/* @} */