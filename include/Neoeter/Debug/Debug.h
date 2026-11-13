#pragma once

#include <cstddef>
#include <utility>

#include <SDL2/SDL.h>

#include <Neoeter/Debug/RowList.h>
#include <Neoeter/Debug/SliderList.h>

namespace Neoeter::Debug
{
#if defined(NEOETER_DEBUG_UI)
	void Initialize();
	void Shutdown();
	void Toggle();
	bool IsVisible();
	bool WantsMouse();
	bool WantsMouseAt(int x, int y);
	bool WantsKeyboard();
	void ProcessEvent(const SDL_Event& event);
	void Draw();

	void AddRowImpl(const char* label, RowList::ValueProvider value);

	template <typename Provider>
	void AddRow(const char* label, Provider&& provider)
	{
		AddRowImpl(label, RowList::ValueProvider(std::forward<Provider>(provider)));
	}

	void AddSliderImpl(const char* label, float minimum, float maximum, SliderList::ValueGetter value, SliderList::ValueSetter setter);

	template <typename Getter, typename Setter>
	void AddSlider(const char* label, float minimum, float maximum, Getter&& getter, Setter&& setter)
	{
		AddSliderImpl(
			label,
			minimum,
			maximum,
			SliderList::ValueGetter(std::forward<Getter>(getter)),
			SliderList::ValueSetter(std::forward<Setter>(setter)));
	}
#else
	inline void Initialize() {}
	inline void Shutdown() {}
	inline void Toggle() {}
	inline bool IsVisible() { return false; }
	inline bool WantsMouse() { return false; }
	inline bool WantsMouseAt(int, int) { return false; }
	inline bool WantsKeyboard() { return false; }
	inline void ProcessEvent(const SDL_Event&) {}
	inline void Draw() {}

	template <typename Provider>
	inline void AddRow(const char*, Provider&&) {}

	template <typename Getter, typename Setter>
	inline void AddSlider(const char*, float, float, Getter&&, Setter&&) {}
#endif
}
