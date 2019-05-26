#pragma once

/// @file

#include "BaseDriver.hpp"

#include <kernel/lib/Types.hpp>

/// @defgroup DriversGroup Drivers group
/// @{

/// @brief Supported serial ports enum
enum SerialPort
{
    COM1_PORT = 0x3F8,
    COM2_PORT = 0x2F8
};

/// @brief Simple serial port driver, used by kprint() and LtDbg (kernel debugger)
class SerialDriver : public BaseDriver
{
public:
    /// @brief Initializes serial port driver
    void Init() override;
        
    /// @brief Reads a byte on a given serial port
    /// @param[in] port The serial port on which the byte is read
    /// @return The read byte
    char ReadByte(SerialPort port);

    /// @brief Writes a byte on a given serial port
    /// @param[in] port The serial port on which the byte is write
    /// @param[in] c The byte to send
    /// @return The written byte
    void WriteByte(SerialPort port, char c);

private:
    /// @brief Initializes a given serial port
    /// @param[in] port The serial port we want to initialize
    void _InitPort(SerialPort port);

    /// @brief Used to know if the given serial port received data
    /// @param[in] port A serial port
    /// @return A value > 0 if the port received data
    int _ReceivedData(SerialPort port);

    /// @brief Used to know if the transmit is empty
    /// @param[in] port A serial port
    /// @return A value != 0 indicates that the transmit is empty, else 0
    int _IsTransmitEmpty(SerialPort port);
};

#ifdef __SERIAL_DRIVER__
SerialDriver gSerialDrv;
#else
extern SerialDriver gSerialDrv;
#endif

/// @}