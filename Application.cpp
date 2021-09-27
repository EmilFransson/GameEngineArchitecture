#include "pch.h"
#include "Application.h"
#include "System.h"
#include "RenderCommand.h"
#include "StackAllocator.h"

#define MEGA 1000000
#define GIGA 1000000000
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

int Application::s_NrOfCubesToPoolAllocate = 0;
static int nrOfCubesToNewAllocate = 0;
static bool useNewAllocator = false;
static bool allocateInChunks = false;

Application::Application() noexcept
	: m_Running{true}
{
	System::Initialize();
	//Default 1280 x 720 window, see function-parameters for dimensions. 
	Window::Initialize(L"GameEngineArchitecture"); 
	m_pImGui = std::make_unique<UI>();
	m_pCubesPool.reserve(1000000);
	m_pCubesNew.reserve(1000000);
	for (uint32_t i{ 0u }; i < 1000000; ++i)
	{
		Cube* pCube = nullptr;
		m_pCubesPool.push_back(pCube);
		Cube* pCube2 = nullptr;
		m_pCubesNew.push_back(pCube2);
	}
}

void Application::Run() noexcept
{
	//Send in the size in bytes
	StackAllocator::CreateAllocator(GIGA / 5);
	while (m_Running)
	{
		static const FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		RenderCommand::ClearBackBuffer(color);
		RenderCommand::ClearDepthBuffer();
		RenderCommand::BindBackBuffer();

		UI::Begin();
		// Windows not part of the dock space goes here:

		//...And ends here.
		UI::BeginDockSpace();
		//Windows part of the dock space goes here:
		RenderPoolAllocatorSettingsPanel<Cube>(m_CubeAllocator, m_pCubesPool);
		RenderNewAllocatorSettingsPanel();

		{
			//Scope could be used for profiling total time.
			if (m_CubeAllocator.IsEnabled())
			{
				if (m_CubeAllocator.IsAllocatingEveryFrame())
				{
					PoolAllocateObjects<Cube>(m_CubeAllocator, m_pCubesPool, m_CubeAllocator.GetNrOfEntitiesAllocatedEveryFrame());
				}
				if (m_CubeAllocator.IsDeallocatingEveryFrame())
				{
					PoolDeallocateObjects<Cube>(m_CubeAllocator, m_pCubesPool, m_CubeAllocator.GetNrOfEntitiesDeallocatedEveryFrame());
				}
			}
		}
		{
			//Scope could be used for profiling total time.
			if (useNewAllocator)
			{
				NewAllocateObjects<Cube>(m_pCubesNew, nrOfCubesToNewAllocate);
				NewDeallocateObjects<Cube>(m_pCubesNew, nrOfCubesToNewAllocate);
			}
		}
		{
			StackAllocateObjects();
		}

		DisplayProfilingResults();
		//...And ends here.
		UI::EndDockSpace();
		//No UI-windows in this part and after! 
		UI::End();
		
		RenderCommand::UnbindRenderTargets();

		if (!Window::OnUpdate())
		{
			//Free up memory:
			m_CubeAllocator.FreeAllMemory(m_pCubesPool);
			StackAllocator::GetInstance()->FreeAllMemory();
			m_Running = false;
		}
	}
}

void Application::DisplayProfilingResults() noexcept
{
	ImGui::Begin("Profiling metrics");
	static bool isGreen = false;
	for (auto& metric : m_ProfileMetrics)
	{
		if (isGreen)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			isGreen = false;
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
			isGreen = true;
		}
		ImGui::Text(std::to_string(metric.Duration).c_str());
		ImGui::SameLine();
		ImGui::Text("ms.");
		ImGui::SameLine();
		ImGui::Text(metric.Name.c_str());
		ImGui::PopStyleColor();
	}
	ImGui::End();
	m_ProfileMetrics.clear();
}

void Application::AllocateCubes(uint32_t nrOfCubes) noexcept
{
	//Stack allocator.
	std::string str = "Application::AllocateCubes (" + std::to_string(nrOfCubes) + ") - StackAllocator";
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
		cubeVector.reserve(nrOfCubes);
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

void Application::RenderNewAllocatorSettingsPanel() noexcept
{
	ImGui::Begin("New-Allocator settings");
	ImGui::Checkbox("Enable", &useNewAllocator);
	if (useNewAllocator)
	{
		ImGui::InputInt("#Cubes to allocate", &nrOfCubesToNewAllocate, 1000);
		if (nrOfCubesToNewAllocate < 0)
		{
			nrOfCubesToNewAllocate = 0;
		}
	}
	ImGui::End();
}

void Application::StackAllocateObjects() noexcept
{
	static int nrOfCubesToStackAllocate = 0;
	ImGui::Begin("StackAllocator Settings");
	static bool pressed = false;
	if (ImGui::Checkbox("Enable", &pressed))
	{
		StackAllocator::GetInstance()->ToggleEnabled();
		if (!StackAllocator::GetInstance()->IsEnabled())
		{
			StackAllocator::GetInstance()->CleanUp();
		}
	}

	if (ImGui::InputInt("#Cubes to allocate.", &nrOfCubesToStackAllocate, 1000))
	{
		if (nrOfCubesToStackAllocate < 0)
			nrOfCubesToStackAllocate = 0;
	}
	ImGui::End();

	if (StackAllocator::GetInstance()->IsEnabled())
	{
		std::string str = __FUNCTION__;
		str.append("'Cube allocation'");
		str.append(" (");
		str.append(std::to_string(nrOfCubesToStackAllocate).c_str());
		str.append(")");
		PROFILE_SCOPE(str);
		for (uint64_t i{ 0u }; i < nrOfCubesToStackAllocate; i++)
		{
			Cube* newCube = StackAllocator::GetInstance()->New<Cube>();

		}
		//Render progressbar before cleanup to visualize usage.
		RenderStackAllocatorProgressBar();
		StackAllocator::GetInstance()->CleanUp();
	}
}

void Application::RenderStackAllocatorProgressBar() noexcept
{
	ImGui::Begin("Stack Allocator memory usage");
	static float progress = 0.0f;

	progress = static_cast<float>(StackAllocator::GetInstance()->GetStackCurrentSize() / static_cast<float>(StackAllocator::GetInstance()->GetStackMaxSize()));
	progress = 1.0f - progress;

	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Bytes free.");
	ImGui::End();
}