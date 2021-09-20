#include "pch.h"
#include "Application.h"
#include "System.h"

Application::Application() noexcept
	: m_Running{true}
{
	System::Initialize();
	//Default 1280 x 720 window, see function-parameters for dimensions. 
	Window::Initialize(L"GameEngineArchitecture"); 
}

void Application::Run() noexcept
{
	while (m_Running)
	{
		if (!Window::OnUpdate())
			m_Running = false;
	}
}