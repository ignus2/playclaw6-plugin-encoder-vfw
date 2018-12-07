/**
 * playclaw6-plugin-encoder-vfw - VFW encoder plugin for PlayClaw 6.
 *
 * Copyright (C) 2018 Balazs OROSZI
 *
 * This file is part of playclaw6-plugin-encoder-vfw.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <stdint.h>
#include <Shlwapi.h>
#include <vfw.h>

#include <string>
#include <memory>

using namespace std;

// TODO: reference additional headers your program requires here
#define JSON_DLL
#include "../../common/jsoncpp/value.h"
#include "../../libs/imgui/imgui.h"
#include "../plugin-api.h"
#include "../plugin-api-helper.h"
#include "../plugin-api-gui.h"
#include "../plugin-encoder-template.h"
#include "../plugin-api-json.h"

