#include <Neoeter/Platform/Base/ImageLoaderCompat.h>
#include <cstring>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#include <Platform/stb_image.h>

namespace
{
const size_t DDS_HEADER_SIZE = 128;

uint32_t ReadU32(const uint8_t* p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

unsigned ChannelShift(uint32_t mask)
{
	unsigned shift = 0;
	while (mask && !(mask & 1u))
	{
		mask >>= 1;
		++shift;
	}
	return shift;
}

unsigned ChannelBits(uint32_t mask)
{
	unsigned bits = 0;
	while (mask)
	{
		++bits;
		mask &= mask - 1u;
	}
	return bits;
}

uint8_t ExpandChannel(uint32_t pixel, uint32_t mask, uint8_t fallback)
{
	if (!mask)
		return fallback;

	const unsigned bits = ChannelBits(mask);
	const uint32_t value = (pixel & mask) >> ChannelShift(mask);
	if (bits >= 8)
		return (uint8_t)(value >> (bits - 8));

	return (uint8_t)((value * 255u) / ((1u << bits) - 1u));
}

bool DecodeDds(const void* data, size_t size, uint32_t* outW, uint32_t* outH, std::vector<uint8_t>& outPixels)
{
	if (!data || size < DDS_HEADER_SIZE)
		return false;

	const uint8_t* bytes = (const uint8_t*)data;
	if (!(bytes[0] == 'D' && bytes[1] == 'D' && bytes[2] == 'S' && bytes[3] == ' '))
		return false;

	if (ReadU32(bytes + 4) != 124)
		return false;

	const uint32_t flags = ReadU32(bytes + 8);
	const uint32_t height = ReadU32(bytes + 12);
	const uint32_t width = ReadU32(bytes + 16);
	const uint32_t pitchOrLinearSize = ReadU32(bytes + 20);
	const uint32_t pixelFlags = ReadU32(bytes + 80);
	const uint32_t fourCC = ReadU32(bytes + 84);
	const uint32_t bitCount = ReadU32(bytes + 88);
	const uint32_t rMask = ReadU32(bytes + 92);
	const uint32_t gMask = ReadU32(bytes + 96);
	const uint32_t bMask = ReadU32(bytes + 100);
	const uint32_t aMask = ReadU32(bytes + 104);

	if (fourCC != 0 || (pixelFlags & 0x40u) == 0)
		return false;
	if (width == 0 || height == 0)
		return false;
	if (bitCount != 8 && bitCount != 16 && bitCount != 24 && bitCount != 32)
		return false;
	if (rMask == 0 || gMask == 0 || bMask == 0)
		return false;

	const size_t bytesPerPixel = bitCount / 8;
	size_t rowPitch = (size_t)width * bytesPerPixel;
	if ((flags & 0x8u) != 0 && pitchOrLinearSize >= rowPitch)
		rowPitch = pitchOrLinearSize;

	if ((size - DDS_HEADER_SIZE) / height < rowPitch)
		return false;

	outPixels.assign((size_t)width * height * 4, 255);
	const uint8_t* src = bytes + DDS_HEADER_SIZE;

	for (uint32_t y = 0; y < height; ++y)
	{
		const uint8_t* row = src + rowPitch * y;
		for (uint32_t x = 0; x < width; ++x)
		{
			const uint8_t* p = row + (size_t)x * bytesPerPixel;
			uint32_t pixel = p[0];
			if (bytesPerPixel > 1)
				pixel |= (uint32_t)p[1] << 8;
			if (bytesPerPixel > 2)
				pixel |= (uint32_t)p[2] << 16;
			if (bytesPerPixel > 3)
				pixel |= (uint32_t)p[3] << 24;

			uint8_t* dst = outPixels.data() + ((size_t)y * width + x) * 4;
			dst[0] = ExpandChannel(pixel, bMask, 255);
			dst[1] = ExpandChannel(pixel, gMask, 255);
			dst[2] = ExpandChannel(pixel, rMask, 255);
			dst[3] = ExpandChannel(pixel, aMask, 255);
		}
	}

	if (outW)
		*outW = width;
	if (outH)
		*outH = height;
	return true;
}
} // namespace

bool Platform_GetImageInfo(const void* data, size_t size, uint32_t* outW, uint32_t* outH, uint32_t* outFormat)
{
	if (!data || size < 18)
		return false;

	// Check DDS magic "DDS " (0x20534444)
	const uint8_t* bytes = (const uint8_t*)data;
	if (bytes[0] == 'D' && bytes[1] == 'D' && bytes[2] == 'S' && bytes[3] == ' ')
	{
		uint32_t h = *(const uint32_t*)(bytes + 12);
		uint32_t w = *(const uint32_t*)(bytes + 16);
		if (outW)
			*outW = w;
		if (outH)
			*outH = h;
		if (outFormat)
		{
			if (size >= 88 && bytes[84] == 'D' && bytes[85] == 'X' && bytes[86] == 'T')
			{
				if (bytes[87] == '1')
					*outFormat = 0x31545844; // D3DFMT_DXT1
				else if (bytes[87] == '3')
					*outFormat = 0x33545844; // D3DFMT_DXT3
				else if (bytes[87] == '5')
					*outFormat = 0x35545844; // D3DFMT_DXT5
				else
					*outFormat = 21; // D3DFMT_A8R8G8B8
			}
			else
			{
				*outFormat = 21; // D3DFMT_A8R8G8B8
			}
		}
		return true;
	}

	// Try stb_image
	int w = 0, h = 0, comp = 0;
	if (stbi_info_from_memory(bytes, (int)size, &w, &h, &comp))
	{
		if (outW)
			*outW = (uint32_t)w;
		if (outH)
			*outH = (uint32_t)h;
		if (outFormat)
			*outFormat = (comp == 4) ? 21 : 20; // D3DFMT_A8R8G8B8 : D3DFMT_R8G8B8
		return true;
	}

	// Try TGA raw header fallback (bytes 12-15)
	uint16_t tgaW = *(const uint16_t*)(bytes + 12);
	uint16_t tgaH = *(const uint16_t*)(bytes + 14);
	if (tgaW > 0 && tgaW <= 4096 && tgaH > 0 && tgaH <= 4096)
	{
		if (outW)
			*outW = tgaW;
		if (outH)
			*outH = tgaH;
		if (outFormat)
			*outFormat = 21;
		return true;
	}

	return false;
}

bool Platform_DecodeImage(const void* data, size_t size, uint32_t* outW, uint32_t* outH, std::vector<uint8_t>& outPixels)
{
	if (!data || size < 4)
		return false;

	if (DecodeDds(data, size, outW, outH, outPixels))
		return true;

	int w = 0, h = 0, comp = 0;
	stbi_uc* pixels = stbi_load_from_memory((const stbi_uc*)data, (int)size, &w, &h, &comp, 4);
	if (!pixels)
		return false;

	if (outW)
		*outW = (uint32_t)w;
	if (outH)
		*outH = (uint32_t)h;

	outPixels.resize(w * h * 4);
	for (int i = 0; i < w * h; ++i)
	{
		outPixels[i * 4 + 0] = pixels[i * 4 + 2]; // B
		outPixels[i * 4 + 1] = pixels[i * 4 + 1]; // G
		outPixels[i * 4 + 2] = pixels[i * 4 + 0]; // R
		outPixels[i * 4 + 3] = pixels[i * 4 + 3]; // A
	}

	stbi_image_free(pixels);
	return true;
}
