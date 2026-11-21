#ifndef __INC_ETERPACK_INLINE_H__
#define __INC_ETERPACK_INLINE_H__

#include <Neoeter/Platform/Platform.h>
#include <Neoeter/Platform/Port.h>
#include <cstring>
#include <cctype>

inline void inlinePathCreate(const char* path)
{
	char dir[256];
	const char* p;
	const char* k;

	p = (strlen(path) > 3 && path[1] == ':') ? (path + 3) : path;

	while (NULL != (k = strchr(p, '/')))
	{
		size_t len = static_cast<size_t>(k - path);
		if (len < sizeof(dir))
		{
			memset(dir, 0, sizeof(dir));
			strncpy(dir, path, len);
			Neoeter::Platform::CreateDirectory(dir);
		}
		p = k + 1;
	}
}

inline void inlineConvertPackFilename(char* name)
{
	char* p = name;

	while (*p)
	{
		if (*p == '\\')
			*p = '/';
		else
			*p = static_cast<char>(tolower(static_cast<unsigned char>(*p)));
		p++;
	}

	static const char legacyRoot[] = "d:/ymir work/";
	const size_t legacyLength = sizeof(legacyRoot) - 1;
	if (strncmp(name, legacyRoot, legacyLength) == 0)
		memmove(name, name + 3, strlen(name + 3) + 1);
}

#endif
