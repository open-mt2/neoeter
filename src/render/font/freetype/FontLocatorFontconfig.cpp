#include "FontLocator.h"

#include <fontconfig/fontconfig.h>

namespace
{
	bool IsPlainAscii(const char* text)
	{
		for (; *text; ++text)
			if ((unsigned char)*text >= 0x80)
				return false;

		return true;
	}
}

bool FontLocator_Fontconfig(const char* family, std::string& path)
{
	path.clear();

	if (!family || !family[0] || !IsPlainAscii(family))
		return false;

	static bool s_inited = FcInit() == FcTrue;
	if (!s_inited)
		return false;

	FcPattern* pattern = FcNameParse((const FcChar8*)family);
	if (!pattern)
		return false;

	FcPatternAddString(pattern, FC_FONTFORMAT, (const FcChar8*)"TrueType");
	FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult result;
	FcPattern* match = FcFontMatch(nullptr, pattern, &result);
	if (match)
	{
		FcChar8* file = nullptr;
		if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch && file)
			path = (const char*)file;
		FcPatternDestroy(match);
	}
	FcPatternDestroy(pattern);

	return !path.empty();
}
