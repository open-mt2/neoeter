#pragma once

#include <cstddef>
#include <utility>

#include <SDL2/SDL.h>

#include <Neoeter/Debug/RowList.h>

namespace Neoeter::Debug
{
#if defined(NEOETER_DEBUG_UI)
	void Initialize();
	void Shutdown();
	void Toggle();
	bool IsVisible();
	bool WantsMouse();
	bool WantsKeyboard();
	void ProcessEvent(const SDL_Event& event);
	void Draw();

	void AddRow(const char* label, const RowList::ValueProvider& value);

	template <typename Provider>
	void AddRow(const char* label, Provider&& provider)
	{
		AddRow(label, RowList::ValueProvider(std::forward<Provider>(provider)));
	}
#else
	inline void Initialize() {}
	inline void Shutdown() {}
	inline void Toggle() {}
	inline bool IsVisible() { return false; }
	inline bool WantsMouse() { return false; }
	inline bool WantsKeyboard() { return false; }
	inline void ProcessEvent(const SDL_Event&) {}
	inline void Draw() {}

	template <typename Provider>
	inline void AddRow(const char*, Provider&&) {}
#endif
}
