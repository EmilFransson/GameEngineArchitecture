#include "pch.h"
#include "Application.h"
#include "System.h"
#include "RenderCommand.h"
#include "StackAllocator.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define PROFILE_FUNC Profiler TOKENPASTE2(profiler, __LINE__) (__FUNCTION__, [&](ProfileMetrics profileMetrics) {m_ProfileMetrics.push_back(std::move(profileMetrics));})
#define PROFILE_SCOPE(scopeName) Profiler TOKENPASTE2(profiler, __LINE__) (scopeName, [&](ProfileMetrics profileMetrics) {m_ProfileMetrics.push_back(std::move(profileMetrics));})

#define MEGA 1000000
#define GIGA 1000000000
Application::Application() noexcept
	: m_Running{true}
{
	System::Initialize();
	//Default 1280 x 720 window, see function-parameters for dimensions. 
	Window::Initialize(L"GameEngineArchitecture"); 
	m_pImGui = std::make_unique<UI>();
}

void Application::Run() noexcept
{
	//Send in the size in bytes
	StackAllocator::CreateAllocator(GIGA);
	while (m_Running)
	{
		static const FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		RenderCommand::ClearBackBuffer(color);
		RenderCommand::ClearDepthBuffer();
		RenderCommand::BindBackBuffer();
		
		AllocateCubes(1000);
		AllocateCubes(10000);
		//AllocateCubes(100000);
		//AllocateCubes(1000000); //Performance LEL

		UI::Begin();
		// Windows not part of the dock space goes here:


		//...And ends here.
		UI::BeginDockSpace();
		//Windows part of the dock space goes here:

		DisplayProfilingResults();
		//...And ends here.
		UI::EndDockSpace();
		//No UI-windows in this part and after! 
		UI::End();
		
		RenderCommand::UnbindRenderTargets();

		if (!Window::OnUpdate())
			m_Running = false;
	}
}

void Application::DisplayProfilingResults() noexcept
{
	ImGui::Begin("Profiling metrics");
	for (auto& metric : m_ProfileMetrics)
	{
		ImGui::Text(std::to_string(metric.Duration).c_str());
		ImGui::SameLine();
		ImGui::Text("ms.");
		ImGui::SameLine();
		ImGui::Text(metric.Name.c_str());
	}
	ImGui::End();
	m_ProfileMetrics.clear();
}

void Application::AllocateCubes(uint32_t nrOfCubes) noexcept
{
	//Poolallocator
	std::string str = "Application::AllocateCubes (" + std::to_string(nrOfCubes) + ") - PoolAllocator";
	{
		PROFILE_SCOPE(str);
		for (uint32_t i{ 0u }; i < nrOfCubes; i++)
		{
			Cube* cube = m_CubeAllocator.New();
			m_CubeAllocator.Delete(cube);
		}
	}
	str = "Application::AllocateCubes (" + std::to_string(nrOfCubes) + ") - malloc";
	{
		PROFILE_SCOPE(str);
		for (uint32_t i{ 0u }; i < nrOfCubes; i++)
		{
			Cube* cube = new Cube();
			delete cube;
		}
	}

	//Stackallocator.
	str = "Application::AllocateCubes (" + std::to_string(nrOfCubes) + ") - StackAllocator";
	{
		PROFILE_SCOPE(str);
		for (uint32_t i{ 0u }; i < nrOfCubes; i++)
		{
			Cube* cube = StackAllocator::GetInstance()->New<Cube>();
		}
		StackAllocator::GetInstance()->CleanUp();
	}
	str = "Application::AllocateCubes (" + std::to_string(nrOfCubes) + ") - Normal stack";
	{
		PROFILE_SCOPE(str);
		std::vector<Cube*> cubeVector;
		for (uint32_t i{ 0u }; i < nrOfCubes; i++)
		{
			cubeVector.push_back(new Cube());
		}
		for (uint32_t i{ 0u }; i < nrOfCubes; i++)
		{
			delete cubeVector[i];
			cubeVector[i] = nullptr;
		}
	}

}
