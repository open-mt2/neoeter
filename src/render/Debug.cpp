#include <Neoeter/Render/StdAfx.h>
#include <Neoeter/Render/MSWindow.h>

#include <Neoeter/Debug/Debug.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

namespace Neoeter::Debug
{
	namespace
	{
		RowList gs_rows;
		SliderList gs_sliders;
		bool gs_visible = false;
		bool gs_initialized = false;
		bool gs_hovered = false;
		float gs_rectMinX = 0.0f;
		float gs_rectMinY = 0.0f;
		float gs_rectMaxX = 0.0f;
		float gs_rectMaxY = 0.0f;
	}

	void Initialize()
	{
		if (gs_initialized)
			return;

		SDL_Window* window = CMSWindow::GetSDLWindow();
		if (!window)
			return;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		if (!ImGui_ImplSDL2_InitForOpenGL(window, CMSWindow::GetGLContext()))
		{
			ImGui::DestroyContext();
			return;
		}

		if (!ImGui_ImplOpenGL3_Init())
		{
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
			return;
		}

		gs_initialized = true;
	}

	void Shutdown()
	{
		if (!gs_initialized)
			return;

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		gs_initialized = false;
	}

	void Toggle()
	{
		gs_visible = !gs_visible;
	}

	bool IsVisible()
	{
		return gs_visible;
	}

	bool WantsMouse()
	{
		return gs_visible && gs_initialized && ImGui::GetIO().WantCaptureMouse;
	}

	bool WantsMouseAt(int x, int y)
	{
		if (!gs_visible || !gs_initialized)
			return false;

		if (ImGui::GetIO().WantCaptureMouse)
			return true;

		const float positionX = static_cast<float>(x);
		const float positionY = static_cast<float>(y);

		return positionX >= gs_rectMinX && positionX < gs_rectMaxX && positionY >= gs_rectMinY && positionY < gs_rectMaxY;
	}

	bool WantsKeyboard()
	{
		return gs_visible && gs_initialized && (gs_hovered || ImGui::GetIO().WantCaptureKeyboard);
	}

	void ProcessEvent(const SDL_Event& event)
	{
		if (gs_initialized)
			ImGui_ImplSDL2_ProcessEvent(&event);
	}

	void AddRowImpl(const char* label, RowList::ValueProvider value)
	{
		gs_rows.Add(label, std::move(value));
	}

	void AddSliderImpl(const char* label, float minimum, float maximum, SliderList::ValueGetter value, SliderList::ValueSetter setter)
	{
		gs_sliders.Add(label, minimum, maximum, std::move(value), std::move(setter));
	}

	void Draw()
	{
		if (!gs_visible || !gs_initialized)
			return;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug", &gs_visible);
		gs_hovered = ImGui::IsWindowHovered();

		const ImVec2 position = ImGui::GetWindowPos();
		const ImVec2 size = ImGui::GetWindowSize();
		gs_rectMinX = position.x;
		gs_rectMinY = position.y;
		gs_rectMaxX = position.x + size.x;
		gs_rectMaxY = position.y + size.y;

		for (std::size_t index = 0; index < gs_rows.Size(); ++index)
			ImGui::Text("%s: %s", gs_rows.Label(index).c_str(), gs_rows.Value(index).c_str());

		for (std::size_t index = 0; index < gs_sliders.Size(); ++index)
		{
			float value = gs_sliders.Value(index);
			ImGui::SetNextItemWidth(180.0f);
			if (ImGui::SliderFloat(
					gs_sliders.Label(index).c_str(),
					&value,
					gs_sliders.Minimum(index),
					gs_sliders.Maximum(index),
					"%.2f"))
				gs_sliders.SetValue(index, value);
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
