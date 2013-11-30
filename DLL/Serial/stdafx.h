/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#pragma once

#define WINVER 0x0600       // Vista and newer
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning(push)
#pragma warning(disable: 4996) // use of deprecated functions
#include <atlstr.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlddx.h>

#pragma warning(disable: 4800) // forcing value to bool 'true' or 'false' (performance warning)
#include "../../lib/ComPortCombo/ComPortCombo.h"
#pragma warning(pop)
#include "../../lib/Utility/Event.h"
#include "../../lib/Utility/HandleTraits.h"
#include "../../lib/Utility/UniqueHandle.h"
#include "../../lib/Utility/WeakHandle.h"
#include <cstdio>
#include <tchar.h>
#include <ctime>
#include <cstdint>

#include <thread>
