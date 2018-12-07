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

#pragma once

class CVideoEncoderVFW : public CPluginEncoderTemplate
{
	void			SetDefaults() override;
	void			LoadFromFile(const Json::Value& config) override;
	void			SaveToFile(Json::Value& config) override;

	BOOL			StartEncoder(unsigned *lpuWidth, unsigned *lpuHeight, unsigned fps1000, GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA* lpInitData) override;
	BOOL			EncodeFrame(const GH_VIDEO_ENCODER_EXTRA_DATA::FRAME_DATA* lpFrameData, GH_VIDEO_ENCODER_EXTRA_DATA::ENCODED_DATA* lpEncodedData);
	BOOL			DrainFrames(GH_VIDEO_ENCODER_EXTRA_DATA::ENCODED_DATA* lpEncodedData);

	void			OnImGuiDraw();

	/// pixelFormat: COLOR_RGB24, etc.
	void			fillBitmapInfoHeader(LPBITMAPINFOHEADER lpbih, unsigned width, unsigned height, int pixelFormat);

	// GUI
	bool  m_refreshImGuiInfo = false;
	CGuiCombobox m_cbPixelFormat;
	std::string m_codecName;
	std::string m_codecDescription;
	std::string m_codecDriver;
	std::string m_codecStateStr;

	// Data, settings
	int m_pixelFormat = GH_VIDEO_ENCODER_EXTRA_DATA::INIT_DATA::COLOR_RGB24;
	uint32_t m_fccHandler = 0;
	std::vector<uint8_t> m_state; ///< Compressor state
	int m_lKey = 1;
	int m_lQ = ICQUALITY_DEFAULT;

	// Encoding
	COMPVARS m_compvars;
	BITMAPINFO m_inputFormat;
	std::vector<uint8_t> m_outputFormatBuf;
	void* m_lpAllocator = 0;

public:
	GH_PLUGIN_VIDEO_ENCODER_CREATE_TEMPLATE_IMGUI(CVideoEncoderVFW);

	CVideoEncoderVFW(void* playclawdata);
	virtual ~CVideoEncoderVFW();
};
