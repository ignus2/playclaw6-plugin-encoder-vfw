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

#include "stdafx.h"
#include "../plugin-api.h"
#include "encoder-vfw.h"

static HMODULE hPluginInstance = 0;

extern "C" __declspec(dllexport) BOOL Plugin_Load()
{
	GH_RegisterPlugin(&CVideoEncoderVFW::plugin_desc);

	return TRUE;
}

extern "C" __declspec(dllexport) void Plugin_Unload()
{
}

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hPluginInstance = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
