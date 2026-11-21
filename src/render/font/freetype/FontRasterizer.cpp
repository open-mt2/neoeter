#include "font/FontRasterizer.h"

#include "font/freetype/FontLocator.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cstddef>
#include <string>

namespace
{
	FT_Library s_library = nullptr;

	bool EnsureLibrary()
	{
		if (s_library)
			return true;

		return FT_Init_FreeType(&s_library) == 0;
	}

	class CFreeTypeRasterizer : public CFontRasterizer
	{
		public:
			CFreeTypeRasterizer(FT_Face face, int pixelSize)
				: m_face(face), m_pixelSize(pixelSize)
			{
			}

			~CFreeTypeRasterizer() override
			{
				FT_Done_Face(m_face);
			}

			bool MeasureGlyph(PlatformDeviceContext, uint16_t, wchar_t codepoint, FontGlyph& glyph) override;
			void DrawGlyph(PlatformDeviceContext, uint32_t* target, int targetWidth, int targetHeight, int x, int y, wchar_t codepoint, uint16_t) override;

		private:
			FT_Face m_face;
			int m_pixelSize;
	};

	bool CFreeTypeRasterizer::MeasureGlyph(PlatformDeviceContext, uint16_t, wchar_t codepoint, FontGlyph& glyph)
	{
		if (FT_Load_Char(m_face, (FT_ULong)codepoint, FT_LOAD_RENDER) != 0)
			return false;

		FT_GlyphSlot slot = m_face->glyph;
		FT_Bitmap& bitmap = slot->bitmap;

		int advance = slot->advance.x >> 6;
		if (advance <= 0)
			advance = (int)bitmap.width + 1;
		if (advance <= 0)
			advance = m_pixelSize / 2;

		int cellWidth = advance;
		if (slot->bitmap_left + (int)bitmap.width > cellWidth)
			cellWidth = slot->bitmap_left + (int)bitmap.width + 1;
		if (cellWidth <= 0)
			cellWidth = m_pixelSize / 2;

		int lineHeight = m_face->size->metrics.height >> 6;
		if (lineHeight <= 0)
			lineHeight = m_pixelSize + 4;

		int ascender = m_face->size->metrics.ascender >> 6;
		if (ascender <= 0)
			ascender = m_pixelSize;

		glyph.cellWidth = cellWidth;
		glyph.cellHeight = lineHeight;
		glyph.left = std::max(0, slot->bitmap_left);
		glyph.top = ascender - slot->bitmap_top;
		glyph.advance = advance;
		glyph.lineHeight = lineHeight;
		glyph.ascender = ascender;
		return true;
	}

	void CFreeTypeRasterizer::DrawGlyph(PlatformDeviceContext, uint32_t* target, int targetWidth, int targetHeight, int x, int y, wchar_t codepoint, uint16_t)
	{
		if (!target || FT_Load_Char(m_face, (FT_ULong)codepoint, FT_LOAD_RENDER) != 0)
			return;

		FT_GlyphSlot slot = m_face->glyph;
		FT_Bitmap& bitmap = slot->bitmap;

		int ascender = m_face->size->metrics.ascender >> 6;
		if (ascender <= 0)
			ascender = m_pixelSize;

		int top = ascender - slot->bitmap_top;
		int left = slot->bitmap_left;
		if (left < 0)
			left = 0;

		for (int row = 0; row < (int)bitmap.rows; ++row)
		{
			int dstY = y + top + row;
			if (dstY < 0 || dstY >= targetHeight)
				continue;

			for (int col = 0; col < (int)bitmap.width; ++col)
			{
				int dstX = x + left + col;
				if (dstX < 0 || dstX >= targetWidth)
					continue;

				uint8_t alpha = bitmap.buffer[row * bitmap.pitch + col];
				if (alpha <= 32)
					continue;

				uint8_t a4 = (alpha + 8) / 17;
				if (a4 > 0)
					target[dstY * targetWidth + dstX] = (a4 << 12) | 0x0FFF;
			}
		}
	}

	// The DIB bars draw with a fixed 12 px sans-serif face, independent of any
	// font texture.
	FT_Face TextFace()
	{
		static FT_Face s_face = nullptr;
		if (s_face || !EnsureLibrary())
			return s_face;

		std::string path = FontLocator_FindFontFile("sans-serif");
		if (!path.empty() && FT_New_Face(s_library, path.c_str(), 0, &s_face) == 0)
			FT_Set_Pixel_Sizes(s_face, 0, 12);

		return s_face;
	}
}

CFontRasterizer* FontRasterizer_Create(const char* faceName, int pixelSize, bool)
{
	if (!EnsureLibrary())
		return nullptr;

	std::string path = FontLocator_FindFontFile(faceName);
	if (path.empty())
		return nullptr;

	FT_Face face = nullptr;
	if (FT_New_Face(s_library, path.c_str(), 0, &face) != 0)
		return nullptr;

	int size = pixelSize > 0 ? pixelSize : 12;
	FT_Set_Pixel_Sizes(face, 0, size);

	return new CFreeTypeRasterizer(face, size);
}

void FontRasterizer_DrawText(PlatformDeviceContext, uint32_t* target, int targetWidth, int targetHeight, int x, int y, const char* text)
{
	FT_Face face = TextFace();
	if (!face || !target || !text)
		return;

	int ascender = face->size->metrics.ascender >> 6;
	int cursorX = x;

	for (size_t i = 0; text[i]; ++i)
	{
		unsigned char ch = (unsigned char)text[i];
		if (FT_Load_Char(face, ch, FT_LOAD_RENDER) != 0)
			continue;

		FT_GlyphSlot slot = face->glyph;
		FT_Bitmap& bitmap = slot->bitmap;

		int top = ascender - slot->bitmap_top;
		int left = slot->bitmap_left;
		if (left < 0)
			left = 0;

		for (int row = 0; row < (int)bitmap.rows; ++row)
		{
			int dstY = y + top + row;
			if (dstY < 0 || dstY >= targetHeight)
				continue;

			for (int col = 0; col < (int)bitmap.width; ++col)
			{
				int dstX = cursorX + left + col;
				if (dstX < 0 || dstX >= targetWidth)
					continue;

				if (bitmap.buffer[row * bitmap.pitch + col] > 32)
					target[dstY * targetWidth + dstX] = 0xFFFFFFFF;
			}
		}

		int advance = slot->advance.x >> 6;
		cursorX += (advance > 0) ? advance : ((int)slot->bitmap.width + 1);
	}
}
