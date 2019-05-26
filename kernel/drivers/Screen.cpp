#include "Screen.hpp"

#include <kernel/drivers/proc_io.hpp>

/// @addgroup DriversGroup
/// @{

static int Line = 1;
static int Column = 1;
static u8 Color = 0x07;
static int CursorEnabled = 0;

CriticalSection ScreenDriver::_criticalSection;

void ScreenDriver::Init()
{
    isInitialized = true;
}

void ScreenDriver::PrintChar(char c)
{
    //_criticalSection.Enter();

	if (c == LF) 
	{
		Line++;
		Column = 0;
	}
	else if (c == CR) 
	{
		Column = 0;
	}
	else 
	{
		if (Column > COLUMNS - 1) 
		{
			Column = 0;
			Line++;
		}

		if (Line > LINES) 
		{
			ScrollUp();
			Line--;
		}

		u8 * screen_ptr = (u8*)SCREEN_PTR + ((Column + (Line * COLUMNS)) * 2);
		*screen_ptr = c;
		*(screen_ptr + 1) = Color;
		Column++;
	}

	if (Column > COLUMNS - 1) {
		Column = 0;
		Line++;
	}

	if (Line > LINES) {
		ScrollUp();
		Line--;
	}

    //_criticalSection.Leave();
}

void ScreenDriver::Clear()
{
	int i = 0;
	u8 * screen_ptr = (u8*)SCREEN_PTR;

    //_criticalSection.Enter();

	for (; i < (LINES*COLUMNS) * 2; i += 2)
		screen_ptr[i] = ' ';
	Line = 0;
	Column = 0;

    //_criticalSection.Leave();
}

void ScreenDriver::SetColor(u8 value)
{
	if (value == WHITE)
		SetColorEx(BLACK, value, 0, 0);
	else
		SetColorEx(BLACK, value, 0, 1);
}

void ScreenDriver::SetColorEx(u8 background, u8 foreground, u8 blink, u8 intensity)
{
    //_criticalSection.Enter();

	Color = 0x0;
	u8 mask = (0x1 << 7);
	Color = (blink & mask);
	mask = (0x7 << 4);
	Color |= (mask & (background << 4));
	mask = (0x1 << 3);
	Color |= (mask & (intensity << 3));
	mask = 0x7;
	Color |= (mask & foreground);

    //_criticalSection.Leave();
}

void ScreenDriver::ScrollUp()
{
	u8 * screen_ptr = (u8*)SCREEN_PTR;
	u8 * screen_end_ptr = (u8*)SCREEN_END_PTR;
	u8 * lastLine_ptr = screen_ptr + ((1 + (LINES * COLUMNS)) * 2);

    //_criticalSection.Enter();

	while (screen_ptr <= screen_end_ptr) {
		if (screen_ptr < lastLine_ptr) {
			*screen_ptr = *(screen_ptr + (COLUMNS * 2));
		}
		else {
			*screen_ptr = 0;
		}
		screen_ptr++;
	}

    //_criticalSection.Leave();
}

void ScreenDriver::SetBackground(u8 color)
{
	u8 * screen_ptr = (u8*)SCREEN_PTR + 1;
	u8 * screen_end_ptr = (u8*)SCREEN_END_PTR;

    //_criticalSection.Enter();

	for (; screen_ptr <= screen_end_ptr; screen_ptr += 2)
		*screen_ptr = (*screen_ptr & 0x8F) | ((color & 0x7) << 4);

    //_criticalSection.Leave();
}

void ScreenDriver::EnableCursor()
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | 13);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);

    CursorEnabled = 1;

	ShowCursor();
}

void ScreenDriver::DisableCursor()
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);

    CursorEnabled = 0;
}

void ScreenDriver::MoveCursor(u8 x, u8 y)
{
	u16 c_pos;

	c_pos = y * COLUMNS + x;

	outb(0x3d4, 0x0f);
	outb(0x3d5, (u8)c_pos & 0xFF);
	outb(0x3d4, 0x0e);
	outb(0x3d5, (u8)(c_pos >> 8) & 0xFF);
}

void ScreenDriver::ShowCursor(void)
{
	MoveCursor(Column, Line);
}

void ScreenDriver::HideCursor(void)
{
	MoveCursor(-1, -1);
}

int ScreenDriver::IsCursorEnabled()
{
	return CursorEnabled;
}

/// @}