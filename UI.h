#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
class UI
{
public:
	UI() noexcept;
	~UI() noexcept;
	static void Begin() noexcept;
	static void BeginDockSpace() noexcept;
	static void End() noexcept;
	static void EndDockSpace() noexcept;
private:
};