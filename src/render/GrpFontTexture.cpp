#include <Neoeter/Render/StdAfx.h>
#include <Neoeter/Render/GrpText.h>
#include <Neoeter/Platform/Base/Stl.h>

#include <Neoeter/Render/Util.h>

#include "font/FontRasterizer.h"

CGraphicFontTexture::CGraphicFontTexture()
{
	Initialize();
}

CGraphicFontTexture::~CGraphicFontTexture()
{
	Destroy();
}

void CGraphicFontTexture::Initialize()
{
	CGraphicTexture::Initialize();
	m_isDirty = false;
	m_bItalic = false;
}

bool CGraphicFontTexture::IsEmpty() const
{
	return m_pFontTextureVector.empty();
}

void CGraphicFontTexture::Destroy()
{
	m_pRasterizer.reset();

	m_dib.Destroy();

	m_lpd3dTexture = NULL;
	CGraphicTexture::Destroy();
	stl_wipe(m_pFontTextureVector);
	m_charInfoMap.clear();

	Initialize();
}

bool CGraphicFontTexture::CreateDeviceObjects()
{
	return true;
}

void CGraphicFontTexture::DestroyDeviceObjects()
{
}

bool CGraphicFontTexture::Create(const char* c_szFontName, int fontSize, bool bItalic)
{
	Destroy();

	strncpy(m_fontName, c_szFontName, sizeof(m_fontName) - 1);
	m_fontSize = fontSize;
	m_bItalic = bItalic;

	m_x = 0;
	m_y = 0;
	m_step = 0;

	uint32_t width = 256, height = 256;
	if (GetMaxTextureWidth() > 512)
		width = 512;
	if (GetMaxTextureHeight() > 512)
		height = 512;

	if (!m_dib.Create(ms_hDC, width, height))
		return false;

	m_pRasterizer.reset(FontRasterizer_Create(c_szFontName, fontSize, bItalic));

	if (!AppendTexture())
		return false;

	return true;
}

bool CGraphicFontTexture::AppendTexture()
{
	CGraphicImageTexture* pNewTexture = new CGraphicImageTexture;

	if (!pNewTexture->Create(m_dib.GetWidth(), m_dib.GetHeight(), D3DFMT_A4R4G4B4))
	{
		delete pNewTexture;
		return false;
	}

	m_pFontTextureVector.push_back(pNewTexture);
	return true;
}

bool CGraphicFontTexture::UpdateTexture()
{
	if (!m_isDirty)
		return true;

	m_isDirty = false;

	CGraphicImageTexture* pFontTexture = m_pFontTextureVector.back();

	if (!pFontTexture)
		return false;

	uint16_t* pwDst;
	int pitch;

	if (!pFontTexture->Lock(&pitch, (void**)&pwDst))
		return false;

	pitch /= 2;

	int width = m_dib.GetWidth();
	int height = m_dib.GetHeight();

	uint32_t* pdwSrc = (uint32_t*)m_dib.GetPointer();

	for (int y = 0; y < height; ++y, pwDst += pitch, pdwSrc += width)
		for (int x = 0; x < width; ++x)
			pwDst[x] = pdwSrc[x];

	pFontTexture->Unlock();
	return true;
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::GetCharacterInfomation(uint16_t codePage, wchar_t keyValue)
{
	TCharacterKey code(codePage, keyValue);

	TCharacterInfomationMap::iterator f = m_charInfoMap.find(code);

	if (m_charInfoMap.end() == f)
	{
		return UpdateCharacterInfomation(code);
	}
	else
	{
		return &f->second;
	}
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::UpdateCharacterInfomation(TCharacterKey code)
{
	wchar_t keyValue = code.second;

	if (keyValue == 0x08)
		keyValue = L' '; // 탭은 공백으로 바꾼다 (아랍 출력시 탭 사용: NAME:\tTEXT -> TEXT\t:NAME 로 전환됨 )

	FontGlyph glyph;
	PlatformDeviceContext hDC = m_dib.GetDCHandle();

	if (!m_pRasterizer || !m_pRasterizer->MeasureGlyph(hDC, code.first, keyValue, glyph))
		return NULL;

	int width = m_dib.GetWidth();
	int height = m_dib.GetHeight();

	if (m_x + glyph.cellWidth >= (width - 1))
	{
		m_y += (m_step + 1);
		m_step = 0;
		m_x = 0;

		if (m_y + glyph.lineHeight >= (height - 1))
		{
			if (!UpdateTexture())
				return NULL;

			if (!AppendTexture())
				return NULL;

			m_y = 0;
			std::memset(m_dib.GetPointer(), 0, (size_t)width * (size_t)height * 4);
		}
	}

	m_pRasterizer->DrawGlyph(hDC, (uint32_t*)m_dib.GetPointer(), width, height, m_x, m_y, keyValue, code.first);

	float rhwidth = 1.0f / float(width);
	float rhheight = 1.0f / float(height);

	TCharacterInfomation& rNewCharInfo = m_charInfoMap[code];

	rNewCharInfo.index = m_pFontTextureVector.size() - 1;
	rNewCharInfo.width = glyph.cellWidth;
	rNewCharInfo.height = glyph.cellHeight;
	rNewCharInfo.left = float(m_x) * rhwidth;
	rNewCharInfo.top = float(m_y) * rhheight;
	rNewCharInfo.right = float(m_x + glyph.cellWidth) * rhwidth;
	rNewCharInfo.bottom = float(m_y + glyph.cellHeight) * rhheight;
	rNewCharInfo.advance = (float)glyph.advance;

	m_x += glyph.cellWidth;

	if (m_step < glyph.lineHeight)
		m_step = glyph.lineHeight;

	m_isDirty = true;

	return &rNewCharInfo;
}

bool CGraphicFontTexture::CheckTextureIndex(uint32_t dwTexture)
{
	if (dwTexture >= m_pFontTextureVector.size())
		return false;

	return true;
}

void CGraphicFontTexture::SelectTexture(uint32_t dwTexture)
{
	assert(CheckTextureIndex(dwTexture));
	m_lpd3dTexture = m_pFontTextureVector[dwTexture]->GetD3DTexture();
}
