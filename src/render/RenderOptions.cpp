#include <Neoeter/Render/StdAfx.h>
#include <Neoeter/Render/RenderOptions.h>

#include <Platform/RenderBackend.h>

namespace Neoeter::Render
{
	void PushLightingDisabled()
	{
		Platform_PushLightingDisabled();
	}

	void PopLightingDisabled()
	{
		Platform_PopLightingDisabled();
	}

	bool IsLightingDisabled()
	{
		return Platform_IsLightingDisabled();
	}
}
