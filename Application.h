#pragma once
#include "Window.h"
#include "UI.h"
class Application
{
public:
	Application() noexcept;
	~Application() noexcept = default;
	void Run() noexcept;
private:
	bool m_Running;
	std::unique_ptr<UI> m_pImGui;
};