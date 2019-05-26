#define __SERIAL_DRIVER__
#include "Serial.hpp"

#include <kernel/drivers/proc_io.hpp>

/// @addgroup DriversGroup
/// @{

void SerialDriver::Init()
{
    _InitPort(COM1_PORT);
    _InitPort(COM2_PORT);

    isInitialized = true;
}

char SerialDriver::ReadByte(SerialPort port)
{
    while (_ReceivedData(port) == 0);

    return inb(port);
}

void SerialDriver::WriteByte(SerialPort port, char c)
{
    while (_IsTransmitEmpty(port) == 0);

    outb(port, c);
}

void SerialDriver::_InitPort(SerialPort port)
{
    outb(port + 1, 0x00); // Deactivates interrupts
    outb(port + 3, 0x80); // Activates DLAB (Divisor Latch Access Bit)
    outb(port + 0, 0x03); // 38400 baud (115200 / 3), lower bits
    outb(port + 1, 0x00); //                          higher bits
    outb(port + 3, 0x03); // 8 bits, no parity, a stop bit
    outb(port + 2, 0xC7);
    outb(port + 4, 0x0B);
    outb(port + 1, 0x01);
}

int SerialDriver::_ReceivedData(SerialPort port)
{
    return inb(port + 5) & 1;
}

int SerialDriver::_IsTransmitEmpty(SerialPort port)
{
    return inb(port + 5) & 0x20;
}

/// @}