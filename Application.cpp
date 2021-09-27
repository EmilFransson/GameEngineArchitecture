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
static bool performTest = false;

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
				if (performTest)
				{

				}
				else
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

	ImGui::Begin("Test Results");
	for (auto& testResult : m_TestResults)
	{
		ImGui::Text(std::to_string(testResult.Duration).c_str());
		ImGui::SameLine();
		ImGui::Text("ms.");
		ImGui::SameLine();
		ImGui::Text(testResult.Name.c_str());
	}
	ImGui::End();
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

	//A lot of small objects.
	if (ImGui::Button("Test Case 1 - Many small objects"))
	{
		for (size_t i = 0; i < 1000; i++)
		{

		}
	}
	if (ImGui::Button("Test Case 2 - Few large objects"))
	{
		for (size_t i = 0; i < 1000; i++)
		{

		}
	}
	if (ImGui::Button("Test Case 3 - Random objects in a random order"))
	{
		for (size_t i = 0; i < 1000; i++)
		{

		}
	}
	ImGui::End();

	if (StackAllocator::GetInstance()->IsEnabled())
	{
		std::string str = __FUNCTION__;
		str.append(" 'Cube allocation & deallocation'");
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

void Application::PerformPoolAllocatorTest1() noexcept
{
	PoolAllocator<Cube> cubeAllocator("Cube Allocator", 400000u);
	std::vector<Cube*> cubes;
	cubes.reserve(400000);
	
	float allocationTimeSum = 0.0f;
	float deallocationTimeSum = 0.0f;
	for (uint64_t i{ 0u }; i < 1000; i++)
	{
		{
			PROFILE_TEST("Cube Pool allocation: Test 1 - 400 000 cubes");
			for (uint64_t j{ 0u }; j < 400000; j++)
			{
				cubes[j] = cubeAllocator.New();
			}
		}
		allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		{
			PROFILE_TEST("Cube Pool deallocation: Test 1 - 400 000 cubes");
			for (uint64_t j{ 0u }; j < 400000; j++)
			{
				cubeAllocator.Delete(cubes[j]);
			}
		}
		deallocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
	}
	ProfileMetrics result = {};
	result.Name = "Cube Pool allocation: Test 1 - 400 000 cubes";
	result.Duration = allocationTimeSum / 1000.0f;
	m_TestResults.push_back(result);
	result.Name = "Cube Pool deallocation: Test 1 - 400 000 cubes";
	result.Duration = deallocationTimeSum / 1000.0f;
	m_TestResults.push_back(result);
	
	m_RepeatedTests.clear();
	result = {}; 
	allocationTimeSum = 0.0f;
	deallocationTimeSum = 0.0f;
	
	for (uint64_t i{ 0u }; i < 1000u; i++)
	{
		{
			PROFILE_TEST("New allocation: Test 1 - 400 000 cubes");
			for (uint64_t j{ 0u }; j < 400000; j++)
			{
				cubes[j] = DBG_NEW Cube;
			}
		}
		allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		{
			PROFILE_TEST("New deallocation: Test 1 - 400 000 cubes");
			for (uint64_t k{ 0u }; k < 400000; k++)
			{
				delete cubes[k];
			}
		}
		deallocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
	}
	result.Name = "Cube New allocation: Test 1 - 400 000 cubes";
	result.Duration = allocationTimeSum / 1000.0f;
	m_TestResults.push_back(result);
	result.Name = "Cube Delete deallocation: Test 1 - 400 000 cubes";
	result.Duration = deallocationTimeSum / 1000.0f;
	m_TestResults.push_back(result);
	
	m_RepeatedTests.clear();
}
