#include "font/FontRasterizer.h"

#include <Neoeter/Platform/Platform.h>
#include <Neoeter/Render/Util.h>

#include <cmath>
#include <cstring>
#include <map>

namespace
{
	class CGdiRasterizer : public CFontRasterizer
	{
		public:
			CGdiRasterizer(const char* faceName, int pixelSize, bool italic);
			~CGdiRasterizer() override;

			bool MeasureGlyph(PlatformDeviceContext dc, uint16_t codePage, wchar_t codepoint, FontGlyph& glyph) override;
			void DrawGlyph(PlatformDeviceContext dc, uint32_t* target, int targetWidth, int targetHeight, int x, int y, wchar_t codepoint, uint16_t codePage) override;

		private:
			HFONT GetFaceFont(uint16_t codePage);

			char m_faceName[LF_FACESIZE];
			int m_pixelSize;
			bool m_italic;
			std::map<uint16_t, HFONT> m_fonts;
	};

	CGdiRasterizer::CGdiRasterizer(const char* faceName, int pixelSize, bool italic)
		: m_pixelSize(pixelSize), m_italic(italic)
	{
		strncpy(m_faceName, faceName ? faceName : "", sizeof(m_faceName) - 1);
		m_faceName[sizeof(m_faceName) - 1] = 0;
	}

	CGdiRasterizer::~CGdiRasterizer()
	{
		for (std::map<uint16_t, HFONT>::iterator it = m_fonts.begin(); it != m_fonts.end(); ++it)
			DeleteObject((HGDIOBJ)it->second);
	}

	HFONT CGdiRasterizer::GetFaceFont(uint16_t codePage)
	{
		std::map<uint16_t, HFONT>::iterator found = m_fonts.find(codePage);
		if (found != m_fonts.end())
			return found->second;

		LOGFONT logFont;
		memset(&logFont, 0, sizeof(LOGFONT));

		logFont.lfHeight = m_pixelSize;
		logFont.lfWeight = FW_NORMAL;
		logFont.lfItalic = (uint8_t)m_italic;
		logFont.lfCharSet = GetCharsetFromCodePage(codePage);
		logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		logFont.lfQuality = ANTIALIASED_QUALITY;
		logFont.lfPitchAndFamily = DEFAULT_PITCH;
		strcpy(logFont.lfFaceName, m_faceName);

		HFONT font = CreateFontIndirect(&logFont);
		m_fonts.insert(std::make_pair(codePage, font));
		return font;
	}

	bool CGdiRasterizer::MeasureGlyph(PlatformDeviceContext dc, uint16_t codePage, wchar_t codepoint, FontGlyph& glyph)
	{
		HFONT font = GetFaceFont(codePage);
		if (font)
			SelectObject(dc, font);

		ABCFLOAT abc;
		SIZE size;

		if (!GetTextExtentPoint32W(dc, &codepoint, 1, &size) || !GetCharABCWidthsFloatW(dc, codepoint, codepoint, &abc))
			return false;

		size.cx = (LONG)abc.abcfB;
		if (abc.abcfA > 0.0f)
			size.cx += (LONG)ceilf(abc.abcfA);
		if (abc.abcfC > 0.0f)
			size.cx += (LONG)ceilf(abc.abcfC);
		size.cx++;

		glyph.cellWidth = size.cx;
		glyph.cellHeight = size.cy;
		glyph.left = 0;
		glyph.top = 0;
		glyph.advance = (int)ceilf(abc.abcfA + abc.abcfB + abc.abcfC);
		glyph.lineHeight = size.cy;
		glyph.ascender = size.cy;
		return true;
	}

	void CGdiRasterizer::DrawGlyph(PlatformDeviceContext dc, uint32_t* target, int targetWidth, int targetHeight, int x, int y, wchar_t codepoint, uint16_t codePage)
	{
		if (!target)
			return;

		FontGlyph glyph;
		if (!MeasureGlyph(dc, codePage, codepoint, glyph))
			return;

		// The atlas conversion below reads the low byte as coverage, so the cell
		// must be white on black; the DIB DC defaults to black on white, which
		// turns every cell into a solid box.
		::SetTextColor(dc, RGB(255, 255, 255));
		::SetBkColor(dc, 0);

		TextOutW(dc, x, y, &codepoint, 1);

		for (int row = 0; row < glyph.cellHeight; ++row)
		{
			int dstY = y + row;
			if (dstY < 0 || dstY >= targetHeight)
				continue;

			for (int col = 0; col < glyph.cellWidth; ++col)
			{
				int dstX = x + col;
				if (dstX < 0 || dstX >= targetWidth)
					continue;

				uint32_t& pixel = target[dstY * targetWidth + dstX];
				pixel = (pixel & 0xff) ? 0xffff : 0;
			}
		}
	}
}

CFontRasterizer* FontRasterizer_Create(const char* faceName, int pixelSize, bool italic)
{
	return new CGdiRasterizer(faceName, pixelSize > 0 ? pixelSize : 12, italic);
}

void FontRasterizer_DrawText(PlatformDeviceContext dc, uint32_t*, int, int, int x, int y, const char* text)
{
	if (!dc || !text)
		return;

	::SetBkColor(dc, 0);
	::TextOutA(dc, x, y, text, (int)strlen(text));
}
