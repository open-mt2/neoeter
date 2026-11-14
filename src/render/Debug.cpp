#include <Neoeter/Render/StdAfx.h>
#include <Neoeter/Render/MSWindow.h>

#include <Neoeter/Debug/Debug.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <string>
#include <vector>

namespace Neoeter::Debug
{
	namespace
	{
		struct SPanel
		{
				std::string menu;
				std::string name;
				PanelDrawFunction draw;
				bool visible;
		};

		RowList gs_rows;
		SliderList gs_sliders;
		ComboList gs_combos;
		std::vector<SPanel> gs_panels;
		bool gs_visible = false;
		bool gs_debugWindowVisible = true;
		bool gs_initialized = false;
		bool gs_hovered = false;
		bool gs_anyItemActive = false;
		std::vector<ImVec4> gs_windowRects;

		bool IsInsideWindowRect(float x, float y)
		{
			for (const ImVec4& rect : gs_windowRects)
			{
				if (x >= rect.x && x < rect.z && y >= rect.y && y < rect.w)
					return true;
			}

			return false;
		}

		bool HasMenu(const std::vector<std::string>& menus, const std::string& menu)
		{
			for (const std::string& entry : menus)
			{
				if (entry == menu)
					return true;
			}

			return false;
		}

		void DrawMenuBar()
		{
			if (!ImGui::BeginMainMenuBar())
				return;

			std::vector<std::string> menus;
			menus.push_back("Debug");

			for (const SPanel& panel : gs_panels)
			{
				if (!HasMenu(menus, panel.menu))
					menus.push_back(panel.menu);
			}

			for (const std::string& menu : menus)
			{
				if (!ImGui::BeginMenu(menu.c_str()))
					continue;

				if (menu == "Debug")
					ImGui::MenuItem("Debug Menu", nullptr, &gs_debugWindowVisible);

				for (SPanel& panel : gs_panels)
				{
					if (panel.menu != menu)
						continue;

					ImGui::MenuItem(panel.name.c_str(), nullptr, &panel.visible);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		void DrawDebugWindow()
		{
			if (!gs_debugWindowVisible)
				return;

			ImGui::Begin("Debug", &gs_debugWindowVisible);

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

			for (std::size_t index = 0; index < gs_combos.Size(); ++index)
			{
				const std::vector<std::string> labels = gs_combos.Labels(index);
				if (labels.empty())
					continue;

				int value = gs_combos.Value(index);
				if (value < 0 || value >= static_cast<int>(labels.size()))
					value = 0;

				ImGui::SetNextItemWidth(180.0f);
				if (ImGui::BeginCombo(gs_combos.Label(index).c_str(), labels[value].c_str()))
				{
					for (int item = 0; item < static_cast<int>(labels.size()); ++item)
					{
						const bool selected = item == value;
						if (ImGui::Selectable(labels[item].c_str(), selected))
							gs_combos.SetValue(index, item);
						if (selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}

			ImGui::End();
		}

	}

	void AddPanelImpl(const char* menu, const char* name, PanelDrawFunction draw, bool visible)
	{
		SPanel panel;
		panel.menu = menu ? menu : "Debug";
		panel.name = name ? name : "";
		panel.draw = std::move(draw);
		panel.visible = visible;
		gs_panels.push_back(std::move(panel));
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

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		ImGui_ImplSDL2_SetMouseCaptureMode(ImGui_ImplSDL2_MouseCaptureMode_Disabled);

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
		if (!gs_visible || !gs_initialized)
			return false;

		const ImVec2 mousePosition = ImGui::GetIO().MousePos;
		return gs_anyItemActive || IsInsideWindowRect(mousePosition.x, mousePosition.y);
	}

	bool WantsMouseAt(int x, int y)
	{
		if (!gs_visible || !gs_initialized)
			return false;

		if (gs_anyItemActive && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			return true;

		return IsInsideWindowRect(static_cast<float>(x), static_cast<float>(y));
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

	void AddComboImpl(const char* label, ComboList::LabelsProvider labels, ComboList::ValueGetter value, ComboList::ValueSetter setter)
	{
		gs_combos.Add(label, std::move(labels), std::move(value), std::move(setter));
	}

	void Draw()
	{
		if (!gs_visible || !gs_initialized)
			return;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		DrawMenuBar();
		DrawDebugWindow();

		for (SPanel& panel : gs_panels)
		{
			if (panel.visible && panel.draw)
				panel.draw(&panel.visible);
		}

		gs_anyItemActive = ImGui::IsAnyItemActive();

		gs_windowRects.clear();
		ImGuiContext* pContext = ImGui::GetCurrentContext();
		if (pContext)
		{
			for (ImGuiWindow* pWindow : pContext->Windows)
			{
				if (!pWindow->WasActive || pWindow->Hidden || pWindow->Size.x <= 0.0f || pWindow->Size.y <= 0.0f)
					continue;

				gs_windowRects.emplace_back(
					pWindow->Pos.x,
					pWindow->Pos.y,
					pWindow->Pos.x + pWindow->Size.x,
					pWindow->Pos.y + pWindow->Size.y);
			}
		}

		const ImVec2 mousePosition = ImGui::GetIO().MousePos;
		gs_hovered = IsInsideWindowRect(mousePosition.x, mousePosition.y);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
