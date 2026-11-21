#pragma once

#include <string>

// Resolves a Windows font face name (e.g. "Tahoma") to a font file on disk.
// Order: the name as a path, $OPENMT2_FONT, fontconfig (family, then
// sans-serif), then a few well-known FHS paths. Results are cached per name.
// Returns an empty string when nothing usable is found.
std::string FontLocator_FindFontFile(const char* fontName);

// Family lookup through fontconfig. A build without fontconfig links the
// fallback translation unit, which returns false and leaves the path and FHS
// lookups in place.
bool FontLocator_Fontconfig(const char* family, std::string& path);
