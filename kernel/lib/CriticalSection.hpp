#pragma once

class CriticalSection
{
public:
    CriticalSection();

    /// @brief Enters in a critical section
    void Enter();

    /// @brief Leaves in a critical section
    void Leave();

//private:
    int _value;
};