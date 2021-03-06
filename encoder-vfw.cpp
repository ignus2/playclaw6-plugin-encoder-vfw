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
#include "encoder-vfw.h"

BOOL IsAvailableVFW()
{
	return TRUE;
}

GH_PLUGIN_VIDEO_ENCODER_DESC_TEMPLATE(CVideoEncoderVFW, "video-encoder-vfw", "VFW", IsAvailableVFW, 0);

///////////////////////////////////

std::string data2hex(const void* data, size_t size)
{
	const char* chars = "0123456789ABCDEF";
	std::string hex(size * 2, '0');
	for (size_t i = 0; i < size; ++i)
	{
		uint8_t c = ((const uint8_t*) data)[i];
		hex[i * 2 + 0] = chars[c >> 4];
		hex[i * 2 + 1] = chars[c & 0xF];
	}
	return hex;
}

uint8_t hex2byte(char hex)
{
	if ('0' <= hex && hex <= '9')
		return hex - '0';
	if ('a' <= hex && hex <= 'f')
		return hex - 'a' + 10;
	if ('A' <= hex && hex <= 'F')
		return hex - 'A' + 10;
	throw std::runtime_error(std::string("Invalid hex character: ") + hex);
}

void hex2data(const std::string& hex, void* data, size_t size)
{
	for (size_t i = 0; i < size && i < hex.length() / 2; ++i)
	{
		uint8_t c1 = hex2byte(hex[i * 2 + 0]);
		uint8_t c2 = hex2byte(hex[i * 2 + 1]);
		((uint8_t*) data)[i] = (c1 << 4) + c2;
	}
}

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

///////////////////////////////////

CVideoEncoderVFW::CVideoEncoderVFW(void* playclawdata)
	: CPluginEncoderTemplate(playclawdata)
{
}

CVideoEncoderVFW::~CVideoEncoderVFW()
{
}

void CVideoEncoderVFW::SetDefaults()
{
	m_pixelFormat = GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA::COLOR_RGB24;
	m_fccHandler = 0;
	m_state.clear();
	m_lKey = 1;
	m_lQ = ICQUALITY_DEFAULT;

	m_cbPixelFormat.m_sName = "##vfw-pixel-format";
	m_cbPixelFormat.m_fWidth = -50; // ?
	m_cbPixelFormat.m_Items.clear();
	m_cbPixelFormat.m_Items.push_back("RGB24");  m_cbPixelFormat.m_ItemsData.push_back(GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA::COLOR_RGB24);
	m_cbPixelFormat.m_Items.push_back("NV12");   m_cbPixelFormat.m_ItemsData.push_back(GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA::COLOR_NV12);
	m_cbPixelFormat.m_Items.push_back("I420");   m_cbPixelFormat.m_ItemsData.push_back(GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA::COLOR_I420);
}

void CVideoEncoderVFW::LoadFromFile(const Json::Value& config)
{
	const Json::Value& vfw = config["vfw"];

	// pixel format
	jsonLoad(m_pixelFormat, vfw["pixelFormat"], false);
	for (size_t i = 0; i < m_cbPixelFormat.m_ItemsData.size(); ++i)
		if (m_pixelFormat == m_cbPixelFormat.m_ItemsData[i])
			m_cbPixelFormat.select(i);

	// fcc
	jsonLoad(m_fccHandler, vfw["fccHandler"], false);

	// state (codec config)
	std::vector<char> state(128 * 1024, '\0');
	jsonLoad(state.data(), state.size(), vfw["state"]);
	std::string stateStr(state.data());
	m_state.resize(stateStr.size() / 2);
	try {
		hex2data(stateStr, m_state.data(), m_state.size());
	} catch (...) {}

	// lKey, lQ
	jsonLoad(m_lKey, vfw["lKey"]);
	jsonLoad(m_lQ, vfw["lQ"]);
}

void CVideoEncoderVFW::SaveToFile(Json::Value& config)
{
	Json::Value& vfw = config["vfw"];
	jsonSave(m_pixelFormat, vfw["pixelFormat"]);
	jsonSave(m_fccHandler, vfw["fccHandler"]);
	jsonSave(data2hex(m_state.data(), m_state.size()).c_str(), vfw["state"]);
	jsonSave(m_lKey, vfw["lKey"]);
	jsonSave(m_lQ, vfw["lQ"]);
}

void CVideoEncoderVFW::fillBitmapInfoHeader(LPBITMAPINFOHEADER lpbih, unsigned width, unsigned height, int pixelFormat)
{
	lpbih->biSize = sizeof(BITMAPINFOHEADER);
	lpbih->biWidth = width;
	lpbih->biHeight = height;
	lpbih->biPlanes = 1;
	switch (m_pixelFormat)
	{
	default:
	case GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA::COLOR_RGB24:
		lpbih->biHeight = -lpbih->biHeight; // negative for top-down DIB
		lpbih->biBitCount = 24;
		lpbih->biCompression = BI_RGB;
		lpbih->biSizeImage = lpbih->biWidth * abs(lpbih->biHeight) * 3;
		break;
	case GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA::COLOR_NV12:
		lpbih->biBitCount = 12;
		lpbih->biCompression = mmioFOURCC('N','V','1','2');
		lpbih->biSizeImage = lpbih->biWidth * abs(lpbih->biHeight) * 12 / 8;
		break;
	case GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA::COLOR_I420:
		lpbih->biBitCount = 12;
		lpbih->biCompression = mmioFOURCC('I','4','2','0');
		lpbih->biSizeImage = lpbih->biWidth * abs(lpbih->biHeight) * 12 / 8;
		break;
	}
}

void CVideoEncoderVFW::OnImGuiDraw()
{
	// Pixel format
	ImGui::Text("Pixel format (select before choosing a compressor!)");
	if (m_cbPixelFormat.draw())
	{
		m_pixelFormat = m_cbPixelFormat.m_ItemsData.at(m_cbPixelFormat.getsel());
	}

	// Button
	bool pressed = ImGui::Button("Choose compressor...");
	if (pressed)
	{
		COMPVARS compvars = {};
		compvars.cbSize = sizeof(compvars);
		compvars.dwFlags = ICMF_COMPVARS_VALID;
		compvars.fccHandler = m_fccHandler;
		BITMAPINFOHEADER bih = {};
		fillBitmapInfoHeader(&bih, 128, 128, m_pixelFormat);
		if (ICCompressorChoose(GetActiveWindow(), 0, &bih, NULL, &compvars, NULL) && compvars.hic)
		{
			// fcc from COMPVARS
			m_fccHandler = compvars.fccHandler;

			// fcc from ICINFO
			//ICINFO icinfo = { sizeof(icinfo) };
			//ICGetInfo(compvars.hic, &icinfo, sizeof(icinfo));
			//m_fccHandler = icinfo.fccHandler;

			// state (codec config)
			m_state.resize(ICGetStateSize(compvars.hic));
			ICGetState(compvars.hic, m_state.data(), m_state.size());

			// lKey, lQ
			m_lKey = compvars.lKey;
			m_lQ = compvars.lQ;

			ICCompressorFree(&compvars);
			m_refreshImGuiInfo = true;
		}
	}

	if (m_refreshImGuiInfo)
	{
		m_refreshImGuiInfo = false;
		m_codecName.clear();
		m_codecDescription.clear();
		m_codecDriver.clear();
		m_codecStateStr.clear();

		HIC hic = ICOpen(ICTYPE_VIDEO, m_fccHandler, ICMODE_COMPRESS);
		if (hic)
		{
			ICINFO icinfo = { sizeof(icinfo) };
			ICGetInfo(hic, &icinfo, sizeof(icinfo));

			m_codecName = utf8_encode(icinfo.szName);
			m_codecDescription = utf8_encode(icinfo.szDescription);
			m_codecDriver = utf8_encode(icinfo.szDriver);
			m_codecStateStr = data2hex(m_state.data(), m_state.size());

			ICClose(hic);
		}
		else
		{
			m_codecName = "Failed to open codec";
		}
	}

	ImGui::Text("Codec info:");
	ImGui::Text(m_codecName.c_str());
	ImGui::Text(m_codecDescription.c_str());
	ImGui::Text(m_codecDriver.c_str());

	//ImGui::Text("Codec state:");
	//ImGui::TextWrapped(m_codecStateStr.c_str());

	ImGui::Text("NOTES:");
	ImGui::TextWrapped("- Codec settings are saved correctly, but cannot be passed back to the codec selection/config dialog, so values there could look different");
	ImGui::TextWrapped("- The RGB24 pixel format is top-down, which some compressors cannot handle (so they won't show up in the list)");
}

void CVideoEncoderVFW::OnImGuiConfigure(void* param)
{
	static int* lpResult = NULL;
	lpResult = (int*)param;

	auto gui_draw = [](void*p)
	{
		CVideoEncoderVFW* data = (CVideoEncoderVFW*)p;
		data->OnImGuiDraw();
	};

	auto gui_result = [](void*p, int res) -> bool
	{
		CVideoEncoderVFW* data = (CVideoEncoderVFW*)p;
		if (lpResult)
			*lpResult = res;
		return true;
	};

	m_refreshImGuiInfo = true;
	GH_GUI_MODAL_CONFIG c = {};
	c.id = GH_LocalizeStr8(m_playclawdata, CVideoEncoderVFW::plugin_desc.szTitleStr);
	c.dlgProc = gui_draw;
	c.dlgResultCB = gui_result;
	c.param = this;
	c.fadeBack = true;
	c.minWidth = 400;
	GH_GuiShowSettings(m_playclawdata, &c);
}

BOOL CVideoEncoderVFW::StartEncoder(unsigned *lpuWidth, unsigned *lpuHeight, unsigned fps1000, GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA* lpInitData)
{
	// Open codec
	HIC hic = ICOpen(ICTYPE_VIDEO, m_fccHandler, ICMODE_COMPRESS);
	if (!hic)
	{
		GH_Print("ERROR: VFW: Cannot open codec");
		return FALSE;
	}

	// Set state
	ICSetState(hic, m_state.data(), m_state.size());

	// Init data
	m_lpAllocator = lpInitData->lpAllocator;
	lpInitData->colorSpace = m_pixelFormat;
	lpInitData->allFrames = true; // ?
	//lpInitData->acceptGPU = false; // ?
	lpInitData->dropLimit = 0; // ?

	// Input format
	// This must be persistent, ICSeqCompressFrameStart() only seems to store a pointer to it
	memset(&m_inputFormat, 0, sizeof(m_inputFormat));
	fillBitmapInfoHeader(&m_inputFormat.bmiHeader, *lpuWidth, *lpuHeight, m_pixelFormat);

	// Determine output format
	DWORD formatSize = ICCompressGetFormatSize(hic, &m_inputFormat);
	m_outputFormatBuf.resize(formatSize);
	LPBITMAPINFO lpbiFormatOut = (LPBITMAPINFO) m_outputFormatBuf.data();
	ICCompressGetFormat(hic, &m_inputFormat, lpbiFormatOut);

	// Begin compression
	memset(&m_compvars, 0, sizeof(m_compvars));
	m_compvars.cbSize = sizeof(m_compvars);
	m_compvars.dwFlags = ICMF_COMPVARS_VALID;
	m_compvars.hic = hic;
	m_compvars.fccType = ICTYPE_VIDEO;
	m_compvars.fccHandler = m_fccHandler;
	m_compvars.lpbiOut = lpbiFormatOut;
	m_compvars.lKey = m_lKey;
	m_compvars.lQ = m_lQ;
	if (!ICSeqCompressFrameStart(&m_compvars, &m_inputFormat))
	{
		GH_Print("ERROR: VFW: Failed to begin compression");
		return FALSE;
	}

	return TRUE;
}

BOOL CVideoEncoderVFW::EncodeFrame(const GH_VIDEO_ENCODER_EXTRA_DATA::FRAME_DATA* lpFrameData, GH_VIDEO_ENCODER_EXTRA_DATA::ENCODED_DATA* lpEncodedData)
{
	// encode
	BOOL fKey = TRUE;
	LONG lSize = m_compvars.lpbiIn->bmiHeader.biSizeImage;
	LPVOID lpData = ICSeqCompressFrame(&m_compvars, 0, lpFrameData->lpData, &fKey, &lSize);
	if (!lpData)
	{
		GH_Print("ERROR: VFW: Error compressing frame");
		return FALSE;
	}

	// new sample
	lpEncodedData->lpMediaSample = GH_VideoEncoderCreateSample(m_lpAllocator, lpData, lSize);

	// fill
	lpEncodedData->timestamp = lpFrameData->timestamp;
	lpEncodedData->framenum = lpFrameData->framenum;
	lpEncodedData->h264 = false;
	lpEncodedData->keyFrame = fKey;
	lpEncodedData->priority = 0;
	lpEncodedData->fccHandler = m_fccHandler;
	if (lpEncodedData->keyFrame)
	{
		lpEncodedData->lpPrivateData = (unsigned char*) m_compvars.lpbiOut;
		lpEncodedData->privateDataSize = m_compvars.lpbiOut->bmiHeader.biSize;
	}

	return TRUE;
}

BOOL CVideoEncoderVFW::DrainFrames(GH_VIDEO_ENCODER_EXTRA_DATA::ENCODED_DATA* lpEncodedData)
{
	ICSeqCompressFrameEnd(&m_compvars);
	ICCompressorFree(&m_compvars);
	return FALSE;
}
