#pragma once

#include <Neoeter/Platform/PortTypes.h>

#include <cstdint>

// Metrics the font atlas needs for one glyph. `cellWidth`/`cellHeight` are the
// atlas slot; `left`/`top` place the rendered bitmap inside it.
struct FontGlyph
{
		int cellWidth;
		int cellHeight;
		int left;
		int top;
		int advance;
		int lineHeight;
		int ascender;
};

// Text rasterisation behind one interface: FreeType on the platforms that ship
// it, GDI on Windows. Call sites never branch on the platform.
class CFontRasterizer
{
	public:
		virtual ~CFontRasterizer() {}

		// Measures one glyph of the loaded face. False when the face cannot
		// produce it, in which case the atlas skips the character.
		virtual bool MeasureGlyph(PlatformDeviceContext dc, uint16_t codePage, wchar_t codepoint, FontGlyph& glyph) = 0;

		// Draws the glyph at the pen position into a 32-bit A4R4G4B4 target.
		virtual void DrawGlyph(PlatformDeviceContext dc, uint32_t* target, int targetWidth, int targetHeight, int x, int y, wchar_t codepoint, uint16_t codePage) = 0;
};

// Loads a face at the given pixel size. Returns nullptr when no face file is
// available; the caller then renders no glyphs for that texture.
CFontRasterizer* FontRasterizer_Create(const char* faceName, int pixelSize, bool italic);

// Draws a text run in white into a 32-bit target. The GDI backend uses the
// font selected into `dc`; the FreeType backend uses a fixed 12 px sans-serif
// face, which is what the DIB bars have always drawn.
void FontRasterizer_DrawText(PlatformDeviceContext dc, uint32_t* target, int targetWidth, int targetHeight, int x, int y, const char* text);
