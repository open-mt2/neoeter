#include <Neoeter/Image/StdAfx.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#if __has_include(<Neoeter/Platform/Base/MappedFile.h>)
#include <Neoeter/Platform/Base/Debug.h>
#include <Neoeter/Platform/Base/MappedFile.h>
#else
#define Tracef(...)                   \
	do {                              \
		fprintf(stdout, __VA_ARGS__); \
	} while (0)
#endif

#include <Neoeter/Image/DXTCImage.h>

// Portable 32-bit compatible DDS header structures & flags (124-byte
// DDSURFACEDESC2)
#ifndef DDSD_CAPS
#define DDSD_CAPS 0x00000001l
#define DDSD_HEIGHT 0x00000002l
#define DDSD_WIDTH 0x00000004l
#define DDSD_PITCH 0x00000008l
#define DDSD_PIXELFORMAT 0x00001000l
#define DDSD_MIPMAPCOUNT 0x00020000l
#define DDSD_LINEARSIZE 0x00080000l
#define DDSD_DEPTH 0x00800000l
#endif

#ifndef DDPF_ALPHAPIXELS
#define DDPF_ALPHAPIXELS 0x00000001l
#define DDPF_ALPHA 0x00000002l
#define DDPF_FOURCC 0x00000004l
#define DDPF_RGB 0x00000040l
#define DDPF_ALPHAPREMULT 0x00008000l
#endif

#pragma pack(push, 1)
typedef struct _DDCOLORKEY_COMPAT
{
		uint32_t dwColorSpaceLowValue;
		uint32_t dwColorSpaceHighValue;
} DDCOLORKEY_COMPAT;

typedef struct _DDSCAPS2_COMPAT
{
		uint32_t dwCaps;
		uint32_t dwCaps2;
		uint32_t dwCaps3;
		uint32_t dwCaps4;
} DDSCAPS2_COMPAT;

typedef struct _DDSURFACEDESC2_COMPAT
{
		uint32_t dwSize;
		uint32_t dwFlags;
		uint32_t dwHeight;
		uint32_t dwWidth;
		union
		{
				int32_t lPitch;
				uint32_t dwLinearSize;
		} DUMMYUNIONNAMEN(1);
		uint32_t dwBackBufferCount;
		union
		{
				uint32_t dwMipMapCount;
				uint32_t dwRefreshRate;
		} DUMMYUNIONNAMEN(2);
		uint32_t dwAlphaBitDepth;
		uint32_t dwReserved;
		uint32_t lpSurface; // 32-bit pointer field in on-disk DDS header
		union
		{
				DDCOLORKEY_COMPAT ddckCKDestOverlay;
				uint32_t dwEmptyFaceColor;
		} DUMMYUNIONNAMEN(3);
		DDCOLORKEY_COMPAT ddckCKDestBlt;
		DDCOLORKEY_COMPAT ddckCKSrcOverlay;
		DDCOLORKEY_COMPAT ddckCKSrcBlt;
		XDDPIXELFORMAT ddpfPixelFormat;
		DDSCAPS2_COMPAT ddsCaps;
		uint32_t dwTextureStage;
} DDSURFACEDESC2;
#pragma pack(pop)

static_assert(sizeof(DDSURFACEDESC2) == 124,
			  "DDSURFACEDESC2 must be exactly 124 bytes to match on-disk DDS "
			  "header format");

struct DXTColBlock
{
		uint16_t col0;
		uint16_t col1;

		// no bit fields - use bytes
		uint8_t row[4];
};

struct DXTAlphaBlockExplicit
{
		uint16_t row[4];
};

struct DXTAlphaBlock3BitLinear
{
		uint8_t alpha0;
		uint8_t alpha1;

		uint8_t stuff[6];
};

// use cast to struct instead of RGBA_MAKE as struct is much
struct Color8888
{
		uint8_t b; // Last one is MSB, 1st is LSB.
		uint8_t g; // order of the output ARGB or BGRA, etc...
		uint8_t r; // change the order of names to change the
		uint8_t a;
};

struct Color565
{
		unsigned nBlue : 5;	 // order of names changes
		unsigned nGreen : 6; //  byte order of output to 32 bit
		unsigned nRed : 5;
};

/////////////////////////////////////
// should be in ddraw.h
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                            \
	((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | \
	 ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif // defined(MAKEFOURCC)

CDXTCImage::CDXTCImage() { Initialize(); }

CDXTCImage::~CDXTCImage() {}

void CDXTCImage::Initialize()
{
	m_nWidth = 0;
	m_nHeight = 0;

	for (int i = 0; i < MAX_MIPLEVELS; ++i)
		m_pbCompBufferByLevels[i] = NULL;
}

void CDXTCImage::Clear()
{
	for (int i = 0; i < MAX_MIPLEVELS; ++i)
		m_bCompVector[i].clear();

	Initialize();
}

bool CDXTCImage::LoadHeaderFromMemory(const uint8_t* c_pbMap, int iSize)
{
	//////////////////////////////////////
	// start reading the file
	// from Microsoft's mssdk D3DIM example "Compress"
	uint32_t dwMagic;

	// Read magic number
	if (iSize < static_cast<int>(sizeof(uint32_t)))
		return false;

	memcpy(&dwMagic, c_pbMap, sizeof(uint32_t));
	c_pbMap += sizeof(uint32_t);
	iSize -= sizeof(uint32_t);

	// Check whether the magic header is correct
	if (dwMagic != MAKEFOURCC('D', 'D', 'S', ' '))
		return false;

	DDSURFACEDESC2 ddsd; // read from dds file

	// Read the surface description
	if (iSize < static_cast<int>(sizeof(DDSURFACEDESC2)))
		return false;

	memcpy(&ddsd, c_pbMap, sizeof(DDSURFACEDESC2));
	c_pbMap += sizeof(DDSURFACEDESC2);
	iSize -= sizeof(DDSURFACEDESC2);

	// Does texture have mipmaps?
	m_bMipTexture = (ddsd.dwMipMapCount > 0) ? TRUE : FALSE;

	// Is it DXTC ?
	// I sure hope pixelformat is valid!
	m_xddPixelFormat.dwFlags = ddsd.ddpfPixelFormat.dwFlags;
	m_xddPixelFormat.dwFourCC = ddsd.ddpfPixelFormat.dwFourCC;
	m_xddPixelFormat.dwSize = ddsd.ddpfPixelFormat.dwSize;
	m_xddPixelFormat.dwRGBBitCount = ddsd.ddpfPixelFormat.dwRGBBitCount;
	m_xddPixelFormat.dwRGBAlphaBitMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	m_xddPixelFormat.dwRBitMask = ddsd.ddpfPixelFormat.dwRBitMask;
	m_xddPixelFormat.dwGBitMask = ddsd.ddpfPixelFormat.dwGBitMask;
	m_xddPixelFormat.dwBBitMask = ddsd.ddpfPixelFormat.dwBBitMask;

	DecodePixelFormat(m_strFormat, &m_xddPixelFormat);

	if (m_CompFormat != PF_DXT1 && m_CompFormat != PF_DXT3 &&
		m_CompFormat != PF_DXT5)
	{
		return false;
	}

	if (ddsd.dwMipMapCount > MAX_MIPLEVELS)
		ddsd.dwMipMapCount = MAX_MIPLEVELS;

	m_nWidth = ddsd.dwWidth;
	m_nHeight = ddsd.dwHeight;
	//!@#
	m_dwMipMapCount = std::max<uint32_t>(1, ddsd.dwMipMapCount);
	m_dwFlags = ddsd.dwFlags;

	if (ddsd.dwFlags & DDSD_PITCH)
	{
		m_lPitch = ddsd.lPitch;
		m_pbCompBufferByLevels[0] = c_pbMap;
	}
	else
	{
		m_lPitch = ddsd.dwLinearSize;

		if (ddsd.dwFlags & DDSD_MIPMAPCOUNT)
		{
			for (uint32_t dwLinearSize = ddsd.dwLinearSize, i = 0; i < m_dwMipMapCount;
				 ++i, dwLinearSize >>= 2)
			{
				m_pbCompBufferByLevels[i] = c_pbMap;
				c_pbMap += dwLinearSize;
			}
		}
		else
		{
			m_pbCompBufferByLevels[0] = c_pbMap;
		}
	}

	return true;
}

bool CDXTCImage::Copy(int miplevel, uint8_t* pbDest, long lDestPitch)
{
	if (!(m_dwFlags & DDSD_MIPMAPCOUNT))
		if (miplevel)
			return false;

	memcpy(pbDest, m_pbCompBufferByLevels[miplevel], m_lPitch >> (miplevel * 2));
	pbDest += lDestPitch;
	return true;
}

void CDXTCImage::Unextract(uint8_t* pbDest, int /*iWidth*/, int /*iHeight*/,
						   int iPitch)
{
	if (!m_pbCompBufferByLevels[0])
		return;

	DXTColBlock* pBlock;
	uint8_t* pPos = (uint8_t*)&m_pbCompBufferByLevels[0][0];
	int xblocks = m_nWidth / 4;
	int yblocks = (m_nHeight / 4) * ((iPitch / m_nWidth) / 2);

	for (int y = 0; y < yblocks; ++y)
	{
		pBlock = (DXTColBlock*)(pPos + y * xblocks * 8);

		memcpy(pbDest, pBlock, xblocks * 8);
		pbDest += xblocks * 8;
	}
}

void CDXTCImage::Decompress(int miplevel, uint32_t* pdwDest)
{
	switch (m_CompFormat)
	{
		case PF_DXT1:
			DecompressDXT1(miplevel, pdwDest);
			break;

		case PF_DXT3:
			DecompressDXT3(miplevel, pdwDest);
			break;

		case PF_DXT5:
			DecompressDXT5(miplevel, pdwDest);
			break;

		case PF_ARGB:
			DecompressARGB(miplevel, pdwDest);
			break;

		case PF_DXT2:
		case PF_DXT4:
		case PF_UNKNOWN:
			break;
	}
}

inline uint32_t Color8888ToDWORD(const Color8888& col)
{
	uint32_t dw;
	memcpy(&dw, &col, sizeof(uint32_t));
	return dw;
}

inline void GetColorBlockColors(DXTColBlock* pBlock, Color8888* col_0,
								Color8888* col_1, Color8888* col_2,
								Color8888* col_3, uint16_t& wrd)
{
	uint16_t rawCol0, rawCol1;
	memcpy(&rawCol0, &pBlock->col0, sizeof(uint16_t));
	memcpy(&rawCol1, &pBlock->col1, sizeof(uint16_t));

	col_0->a = 0xff;
	col_0->r = (uint8_t)(((rawCol0 >> 11) & 0x1f) << 3); // shift to full precision
	col_0->g = (uint8_t)(((rawCol0 >> 5) & 0x3f) << 2);
	col_0->b = (uint8_t)((rawCol0 & 0x1f) << 3);

	col_1->a = 0xff;
	col_1->r = (uint8_t)(((rawCol1 >> 11) & 0x1f) << 3); // shift to full precision
	col_1->g = (uint8_t)(((rawCol1 >> 5) & 0x3f) << 2);
	col_1->b = (uint8_t)((rawCol1 & 0x1f) << 3);

	if (rawCol0 > rawCol1)
	{
		// Four-color block: derive the other two colors.
		// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
		wrd = (uint16_t)(((uint16_t)col_0->r * 2 + (uint16_t)col_1->r) / 3);
		col_2->r = (uint8_t)wrd;

		wrd = (uint16_t)(((uint16_t)col_0->g * 2 + (uint16_t)col_1->g) / 3);
		col_2->g = (uint8_t)wrd;

		wrd = (uint16_t)(((uint16_t)col_0->b * 2 + (uint16_t)col_1->b) / 3);
		col_2->b = (uint8_t)wrd;
		col_2->a = 0xff;

		wrd = (uint16_t)(((uint16_t)col_0->r + (uint16_t)col_1->r * 2) / 3);
		col_3->r = (uint8_t)wrd;

		wrd = (uint16_t)(((uint16_t)col_0->g + (uint16_t)col_1->g * 2) / 3);
		col_3->g = (uint8_t)wrd;

		wrd = (uint16_t)(((uint16_t)col_0->b + (uint16_t)col_1->b * 2) / 3);
		col_3->b = (uint8_t)wrd;
		col_3->a = 0xff;
	}
	else
	{
		// Three-color block: derive the other color.
		// 00 = color_0,  01 = color_1,  10 = color_2,
		// 11 = transparent.
		wrd = (uint16_t)(((uint16_t)col_0->r + (uint16_t)col_1->r) / 2);
		col_2->r = (uint8_t)wrd;
		wrd = (uint16_t)(((uint16_t)col_0->g + (uint16_t)col_1->g) / 2);
		col_2->g = (uint8_t)wrd;
		wrd = (uint16_t)(((uint16_t)col_0->b + (uint16_t)col_1->b) / 2);
		col_2->b = (uint8_t)wrd;
		col_2->a = 0xff;

		col_3->r = 0x00; // random color to indicate alpha
		col_3->g = 0x00;
		col_3->b = 0x00;
		col_3->a = 0x00;
	}
}

inline void DecodeColorBlock(uint32_t* pImPos, DXTColBlock* pColorBlock, int width,
							 uint32_t* col_0, uint32_t* col_1, uint32_t* col_2,
							 uint32_t* col_3)
{
	// width is width of image in pixels
	uint32_t bits;
	int y, n;

	uint32_t c0, c1, c2, c3;
	memcpy(&c0, col_0, sizeof(uint32_t));
	memcpy(&c1, col_1, sizeof(uint32_t));
	memcpy(&c2, col_2, sizeof(uint32_t));
	memcpy(&c3, col_3, sizeof(uint32_t));

	// bit masks = 00000011, 00001100, 00110000, 11000000
	const uint32_t masks[] = {3, 12, 3 << 4, 3 << 6};
	const int shift[] = {0, 2, 4, 6};

	// r steps through lines in y
	for (y = 0; y < 4;
		 ++y, pImPos += width - 4) // no width * 4 as uint32_t ptr inc will * 4
	{
		// n steps through pixels
		for (n = 0; n < 4; ++n)
		{
			bits = pColorBlock->row[y] & masks[n];
			bits >>= shift[n];

			switch (bits)
			{
				case 0:
					*pImPos = c0;
					pImPos++; // increment to next uint32_t
					break;

				case 1:
					*pImPos = c1;
					pImPos++;
					break;

				case 2:
					*pImPos = c2;
					pImPos++;
					break;

				case 3:
					*pImPos = c3;
					pImPos++;
					break;

				default:
					Tracef("Your logic is jacked! bits == 0x%x\n", bits);
					pImPos++;
					break;
			}
		}
	}
}

inline void DecodeAlphaExplicit(uint32_t* pImPos,
								DXTAlphaBlockExplicit* pAlphaBlock, int width,
								uint32_t alphazero)
{
	// decodes to 32 bit format only
	int row, pix;

	uint16_t wrd;

	Color8888 col;
	col.r = col.g = col.b = 0;

	for (row = 0; row < 4; row++, pImPos += width - 4)
	{
		memcpy(&wrd, &pAlphaBlock->row[row], sizeof(uint16_t));

		for (pix = 0; pix < 4; ++pix)
		{
			// zero the alpha bits of image pixel
			*pImPos &= alphazero;

			col.a = (uint8_t)(wrd & 0x000f); // get only low 4 bits
			col.a =
				(uint8_t)(col.a |
						  (col.a << 4)); // This allows max 4 bit alpha to be 0xff alpha

			*pImPos |=
				Color8888ToDWORD(col); // or the bits into the prev. nulled alpha

			wrd >>= 4; // move next bits to lowest 4

			pImPos++; // move to next pixel in the row
		}
	}
}

static uint8_t gBits[4][4];
static uint16_t gAlphas[8];
static Color8888 gACol[4][4];

inline void DecodeAlpha3BitLinear(uint32_t* pImPos,
								  DXTAlphaBlock3BitLinear* pAlphaBlock,
								  int width, uint32_t alphazero)
{
	gAlphas[0] = pAlphaBlock->alpha0;
	gAlphas[1] = pAlphaBlock->alpha1;

	// 8-alpha or 6-alpha block?
	if (gAlphas[0] > gAlphas[1])
	{
		// 8-alpha block:  derive the other 6 alphas.
		// 000 = alpha_0, 001 = alpha_1, others are interpolated
		gAlphas[2] = (uint16_t)((6 * gAlphas[0] + gAlphas[1]) / 7);		// Bit code 010
		gAlphas[3] = (uint16_t)((5 * gAlphas[0] + 2 * gAlphas[1]) / 7); // Bit code 011
		gAlphas[4] = (uint16_t)((4 * gAlphas[0] + 3 * gAlphas[1]) / 7); // Bit code 100
		gAlphas[5] = (uint16_t)((3 * gAlphas[0] + 4 * gAlphas[1]) / 7); // Bit code 101
		gAlphas[6] = (uint16_t)((2 * gAlphas[0] + 5 * gAlphas[1]) / 7); // Bit code 110
		gAlphas[7] = (uint16_t)((gAlphas[0] + 6 * gAlphas[1]) / 7);		// Bit code 111
	}
	else
	{
		// 6-alpha block:  derive the other alphas.
		// 000 = alpha_0, 001 = alpha_1, others are interpolated
		gAlphas[2] = (uint16_t)((4 * gAlphas[0] + gAlphas[1]) / 5);		// Bit code 010
		gAlphas[3] = (uint16_t)((3 * gAlphas[0] + 2 * gAlphas[1]) / 5); // Bit code 011
		gAlphas[4] = (uint16_t)((2 * gAlphas[0] + 3 * gAlphas[1]) / 5); // Bit code 100
		gAlphas[5] = (uint16_t)((gAlphas[0] + 4 * gAlphas[1]) / 5);		// Bit code 101
		gAlphas[6] = 0;													// Bit code 110
		gAlphas[7] = 255;												// Bit code 111
	}

	// Decode 3-bit fields into array of 16 BYTES with same value
	const uint32_t mask = 0x00000007; // bits = 00 00 01 11
	uint32_t bits = 0;
	memcpy(&bits, &(pAlphaBlock->stuff[0]), 3);

	gBits[0][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][3] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][3] = (uint8_t)(bits & mask);

	// now for last two rows:
	bits = 0;
	memcpy(&bits, &(pAlphaBlock->stuff[3]), 3); // last 3 bytes

	gBits[2][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][3] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][3] = (uint8_t)(bits & mask);

	// decode the codes into alpha values
	int row, pix;

	for (row = 0; row < 4; ++row)
	{
		for (pix = 0; pix < 4; ++pix)
		{
			gACol[row][pix].a = (uint8_t)gAlphas[gBits[row][pix]];

			assert(gACol[row][pix].r == 0);
			assert(gACol[row][pix].g == 0);
			assert(gACol[row][pix].b == 0);
		}
	}

	// Write out alpha values to the image bits
	for (row = 0; row < 4; ++row, pImPos += width - 4)
	{
		for (pix = 0; pix < 4; ++pix)
		{
			// zero the alpha bits of image pixel
			*pImPos &= alphazero;
			*pImPos |= Color8888ToDWORD(
				gACol[row][pix]); // or the bits into the prev. nulled alpha
			pImPos++;
		}
	}
}

void CDXTCImage::DecompressDXT1(int miplevel, uint32_t* pdwDest)
{
	int xblocks, yblocks;
	uint32_t nWidth = m_nWidth >> miplevel;
	uint32_t nHeight = m_nHeight >> miplevel;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	int x, y;
	uint32_t* pBase = (uint32_t*)pdwDest;
	uint16_t* pPos = (uint16_t*)&m_bCompVector[miplevel][0]; // pos in compressed data
	uint32_t* pImPos;

	DXTColBlock* pBlock;

	Color8888 col_0, col_1, col_2, col_3;
	uint16_t wrd;

	for (y = 0; y < yblocks; ++y)
	{
		// 8 bytes per block
		pBlock = (DXTColBlock*)((uintptr_t)pPos + y * xblocks * 8);

		for (x = 0; x < xblocks; ++x, ++pBlock)
		{
			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			pImPos = (uint32_t*)((uintptr_t)pBase + x * 16 + (y * 4) * nWidth * 4);
			DecodeColorBlock(pImPos, pBlock, nWidth, (uint32_t*)&col_0, (uint32_t*)&col_1,
							 (uint32_t*)&col_2, (uint32_t*)&col_3);
		}
	}
}

void CDXTCImage::DecompressDXT3(int miplevel, uint32_t* pdwDest)
{
	int xblocks, yblocks;
	uint32_t nWidth = m_nWidth >> miplevel;
	uint32_t nHeight = m_nHeight >> miplevel;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	int x, y;
	uint32_t* pBase = (uint32_t*)pdwDest;
	uint16_t* pPos = (uint16_t*)&m_bCompVector[miplevel][0]; // pos in compressed data
	uint32_t* pImPos;										 // pos in decompressed data

	DXTColBlock* pBlock;
	DXTAlphaBlockExplicit* pAlphaBlock;

	Color8888 col_0, col_1, col_2, col_3;
	uint16_t wrd;

	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;

	uint32_t alphazero = Color8888ToDWORD(col_0);

	for (y = 0; y < yblocks; ++y)
	{
		// 16 bytes per block (8 alpha + 8 color)
		pBlock = (DXTColBlock*)((uintptr_t)(pPos + y * xblocks * 8));

		for (x = 0; x < xblocks; ++x, ++pBlock)
		{
			// Get alpha block
			pAlphaBlock = (DXTAlphaBlockExplicit*)pBlock;

			// Get color block & colors
			pBlock++;
			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			// Decode the color block into the bitmap bits
			pImPos = (uint32_t*)((uintptr_t)pBase + x * 16 + (y * 4) * nWidth * 4);

			DecodeColorBlock(pImPos, pBlock, nWidth, (uint32_t*)&col_0, (uint32_t*)&col_1,
							 (uint32_t*)&col_2, (uint32_t*)&col_3);

			// Overwrite the previous alpha bits with the alpha block info
			DecodeAlphaExplicit(pImPos, pAlphaBlock, nWidth, alphazero);
		}
	}
}

void CDXTCImage::DecompressDXT5(int level, uint32_t* pdwDest)
{
	int xblocks, yblocks;
	uint32_t nWidth = m_nWidth >> level;
	uint32_t nHeight = m_nHeight >> level;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	int x, y;

	uint32_t* pBase = (uint32_t*)pdwDest;
	uint16_t* pPos = (uint16_t*)&m_bCompVector[level][0]; // pos in compressed data
	uint32_t* pImPos;									  // pos in decompressed data

	DXTColBlock* pBlock;
	DXTAlphaBlock3BitLinear* pAlphaBlock;

	Color8888 col_0, col_1, col_2, col_3;
	uint16_t wrd;

	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;
	uint32_t alphazero = Color8888ToDWORD(col_0);

	for (y = 0; y < yblocks; ++y)
	{
		// 16 bytes per block (8 alpha + 8 color)
		pBlock = (DXTColBlock*)((uintptr_t)(pPos + y * xblocks * 8));

		for (x = 0; x < xblocks; ++x, ++pBlock)
		{
			// Get alpha block
			pAlphaBlock = (DXTAlphaBlock3BitLinear*)pBlock;

			// Get color block & colors
			pBlock++;

			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			// Decode the color block into the bitmap bits
			pImPos = (uint32_t*)((uintptr_t)pBase + x * 16 + (y * 4) * nWidth * 4);

			DecodeColorBlock(pImPos, pBlock, nWidth, (uint32_t*)&col_0, (uint32_t*)&col_1,
							 (uint32_t*)&col_2, (uint32_t*)&col_3);

			// Overwrite the previous alpha bits with the alpha block info
			DecodeAlpha3BitLinear(pImPos, pAlphaBlock, nWidth, alphazero);
		}
	}
}

void CDXTCImage::DecompressARGB(int level, uint32_t* pdwDest)
{
	uint32_t lPitch = m_lPitch >> (level * 2);
	memcpy(pdwDest, &m_bCompVector[level][0], lPitch);
}

static uint16_t GetNumberOfBits(uint32_t dwMask)
{
	uint16_t wBits;
	for (wBits = 0; dwMask; wBits++)
		dwMask = (dwMask & (dwMask - 1));

	return wBits;
}

void CDXTCImage::DecodePixelFormat(char* strPixelFormat,
								   XDDPIXELFORMAT* pxddpf)
{
	switch (pxddpf->dwFourCC)
	{
		case 0:
		{
			// This dds texture isn't compressed so write out ARGB format
			uint16_t a = GetNumberOfBits(pxddpf->dwRGBAlphaBitMask);
			uint16_t r = GetNumberOfBits(pxddpf->dwRBitMask);
			uint16_t g = GetNumberOfBits(pxddpf->dwGBitMask);
			uint16_t b = GetNumberOfBits(pxddpf->dwBBitMask);

			_snprintf(strPixelFormat, 31, "ARGB-%d%d%d%d%s", a, r, g, b,
					  pxddpf->dwBBitMask & DDPF_ALPHAPREMULT ? "-premul" : "");
			m_CompFormat = PF_ARGB;
		}
		break;

		case MAKEFOURCC('D', 'X', 'T', '1'):
			strncpy(strPixelFormat, "DXT1", 31);
			m_CompFormat = PF_DXT1;
			break;

		case MAKEFOURCC('D', 'X', 'T', '2'):
			strncpy(strPixelFormat, "DXT2", 31);
			m_CompFormat = PF_DXT2;
			break;

		case MAKEFOURCC('D', 'X', 'T', '3'):
			strncpy(strPixelFormat, "DXT3", 31);
			m_CompFormat = PF_DXT3;
			break;

		case MAKEFOURCC('D', 'X', 'T', '4'):
			strncpy(strPixelFormat, "DXT4", 31);
			m_CompFormat = PF_DXT4;
			break;

		case MAKEFOURCC('D', 'X', 'T', '5'):
			strncpy(strPixelFormat, "DXT5", 31);
			m_CompFormat = PF_DXT5;
			break;

		default:
			strcpy(strPixelFormat, "Format Unknown");
			m_CompFormat = PF_UNKNOWN;
			break;
	}
}
