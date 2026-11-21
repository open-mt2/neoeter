#include "FontLocator.h"

#include <Neoeter/Platform/Base/Debug.h>

#include <cstdlib>
#include <map>
#include <mutex>
#include <string>
#include <unistd.h>

namespace
{
bool IsReadable(const std::string& path)
{
	return !path.empty() && access(path.c_str(), R_OK) == 0;
}

std::string Resolve(const char* fontName)
{
	if (fontName && fontName[0] && IsReadable(fontName))
		return fontName;

	if (const char* env = getenv("OPENMT2_FONT"))
		if (IsReadable(env))
			return env;

	std::string path;
	if (fontName && fontName[0] && FontLocator_Fontconfig(fontName, path) && IsReadable(path))
		return path;

	path.clear();
	if (FontLocator_Fontconfig("sans-serif", path) && IsReadable(path))
		return path;

	static const char* s_fontPaths[] = {
		"/usr/share/fonts/TTF/DejaVuSans.ttf",
		"/usr/share/fonts/noto/NotoSans-Regular.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
		"/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
		"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSans.ttf",
		nullptr};
	for (int i = 0; s_fontPaths[i]; ++i)
		if (IsReadable(s_fontPaths[i]))
			return s_fontPaths[i];

	return std::string();
}
}

std::string FontLocator_FindFontFile(const char* fontName)
{
	static std::mutex s_lock;
	static std::map<std::string, std::string> s_cache;

	std::string key = fontName ? fontName : "";
	std::lock_guard<std::mutex> guard(s_lock);

	std::map<std::string, std::string>::iterator found = s_cache.find(key);
	if (found != s_cache.end())
		return found->second;

	std::string path = Resolve(fontName);
	if (path.empty())
		TraceError("No font file found for '%s'; set OPENMT2_FONT to a .ttf path", key.c_str());
	else
		Tracef("Font '%s' -> %s\n", key.c_str(), path.c_str());

	s_cache[key] = path;
	return path;
}
