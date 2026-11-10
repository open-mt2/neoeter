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
		bool gs_visible = false;
		bool gs_initialized = false;
		bool gs_hovered = false;
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

	void Draw()
	{
		if (!gs_visible || !gs_initialized)
			return;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug", &gs_visible);
		gs_hovered = ImGui::IsWindowHovered();
		for (std::size_t index = 0; index < gs_rows.Size(); ++index)
			ImGui::Text("%s: %s", gs_rows.Label(index).c_str(), gs_rows.Value(index).c_str());
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
