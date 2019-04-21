#pragma once

//replace auto_ptr used by kiwi
#define auto_ptr unique_ptr

#include <kiwi.h>

#undef auto_ptr
