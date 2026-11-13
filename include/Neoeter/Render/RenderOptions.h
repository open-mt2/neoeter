#pragma once

namespace Neoeter::Render
{
	void PushLightingDisabled();
	void PopLightingDisabled();
	bool IsLightingDisabled();

	class LightingDisabledScope
	{
		public:
			LightingDisabledScope() { PushLightingDisabled(); }
			~LightingDisabledScope() { PopLightingDisabled(); }
			LightingDisabledScope(const LightingDisabledScope&) = delete;
			LightingDisabledScope& operator=(const LightingDisabledScope&) = delete;
	};
}
