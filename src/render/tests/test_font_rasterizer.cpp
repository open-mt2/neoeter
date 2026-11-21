#include <support/TestAssert.h>

#include "font/FontRasterizer.h"

#include <cstddef>
#include <cstdint>
#include <vector>

// The font seam: a face measures and rasterises glyphs into an A4R4G4B4
// target, and text draws into a DIB buffer, without the caller knowing whether
// FreeType or GDI is behind it. The fake below is the second implementation
// the interface has to accept; the real backend is exercised when the host has
// a font file.

namespace
{
class CFakeRasterizer : public CFontRasterizer
{
	public:
		bool MeasureGlyph(PlatformDeviceContext, uint16_t, wchar_t, FontGlyph& glyph) override
		{
			glyph.cellWidth = 6;
			glyph.cellHeight = 8;
			glyph.left = 1;
			glyph.top = 2;
			glyph.advance = 7;
			glyph.lineHeight = 8;
			glyph.ascender = 6;
			return true;
		}

		void DrawGlyph(PlatformDeviceContext, uint32_t* target, int targetWidth, int targetHeight, int x, int y, wchar_t, uint16_t) override
		{
			if (x >= 0 && x < targetWidth && y >= 0 && y < targetHeight)
				target[y * targetWidth + x] = 0x0FFF;
		}
};
}

int main()
{
	TEST_SECTION("The seam accepts a second implementation");

	CFakeRasterizer fake;
	FontGlyph glyph{};
	TEST_ASSERT(fake.MeasureGlyph(nullptr, 0, L'X', glyph));
	TEST_ASSERT(glyph.advance == 7);

	std::vector<uint32_t> atlas(16 * 16, 0);
	fake.DrawGlyph(nullptr, atlas.data(), 16, 16, 3, 4, L'X', 0);
	TEST_ASSERT(atlas[4 * 16 + 3] == 0x0FFF);
	TEST_PASS("measure and draw travel through the interface");

	TEST_SECTION("The loaded backend measures and rasterises");

	CFontRasterizer* face = FontRasterizer_Create("sans-serif", 16, false);
	if (!face)
	{
		TEST_INFO("no font file on this host; the backend raster path is not exercised");
		return 0;
	}

	TEST_ASSERT(face->MeasureGlyph(nullptr, 0, L'A', glyph));
	TEST_ASSERT(glyph.advance > 0);
	TEST_ASSERT(glyph.cellWidth > 0);
	TEST_ASSERT(glyph.cellHeight > 0);

	std::vector<uint32_t> target(64 * 64, 0);
	face->DrawGlyph(nullptr, target.data(), 64, 64, 8, 8, L'A', 0);

	bool painted = false;
	for (size_t i = 0; i < target.size(); ++i)
		if (target[i] != 0)
			painted = true;

	TEST_ASSERT(painted);
	TEST_PASS("the backend writes A4R4G4B4 pixels");

	TEST_SECTION("Text draws through the same seam");

	std::vector<uint32_t> bar(64 * 32, 0);
	FontRasterizer_DrawText(nullptr, bar.data(), 64, 32, 2, 4, "openMT2");

	painted = false;
	for (size_t i = 0; i < bar.size(); ++i)
		if (bar[i] != 0)
			painted = true;

	TEST_ASSERT(painted);
	TEST_PASS("DrawText paints a run into the buffer");

	delete face;
	return 0;
}
