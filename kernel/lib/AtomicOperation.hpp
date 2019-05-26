#pragma once

#include "StdLib.hpp"

extern "C" void _acquire_lock(int * value);

extern "C" void _release_lock(int * value);