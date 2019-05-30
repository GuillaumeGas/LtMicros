#pragma once

/// @file

/// @addgroup Debug Debug group
/// @{

#define COMMANDS_LIST                     \
	COMMAND(CMD_CONNECT,     "connect")   \
	COMMAND(CMD_STEP,        "p")         \
	COMMAND(CMD_CONTINUE,    "c")         \
	COMMAND(CMD_QUIT,        "q")         \
	COMMAND(CMD_REGISTERS,   "r")		  \
	COMMAND(CMD_DISASS,      "d")		  \
	COMMAND(CMD_STACK_TRACE, "s")		  \
	COMMAND(CMD_MEMORY,      "m")		  \
	COMMAND(CMD_BP,          "bp")        \
	COMMAND(CMD_BL,          "bl")        \
	COMMAND(CMD_UNKNOWN,     "<unknown>") \
	COMMAND(CMD_END,         "<end>" )    \

enum CommandId
{
#define COMMAND(name, _) name,
#define COMMAND_KEYWORD(x, y) COMMAND(x, y)
    COMMANDS_LIST
#undef COMMAND
#undef COMMAND_KEYWORD
} typedef CommandId;

/// @}