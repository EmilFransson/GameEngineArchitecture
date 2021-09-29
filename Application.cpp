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

void StackAllocatorTestOne()
{
	if (ImGui::Button("Test Case 1 - Many small objects"))
	{
		const size_t n = 400000;
		const size_t testCases = 1000;

		//Stack Allocator
		for (size_t i = 0; i < testCases; i++)
		{
			for (size_t j = 0; j < n; j++)
			{
				StackAllocator::GetInstance()->New<Cube>();
			}
			StackAllocator::GetInstance()->CleanUp();
		}

		//Normal new/delete allocation.
		for (size_t i = 0; i < testCases; i++)
		{
			std::vector<Cube*> cubeArray;
			for (size_t j = 0; j < n; j++)
			{
				cubeArray.push_back(StackAllocator::GetInstance()->New<Cube>());
			}
			for (size_t j = n; j > 0; j--)
			{
				delete cubeArray[j - 1];
			}
		}
	}
}

void StackAllocatorTestTwo()
{
	if (ImGui::Button("Test Case 2 - Few large objects"))
	{
		const size_t n = 4000;
		const size_t testCases = 1000;

		//Stack Allocator
		for (size_t i = 0; i < testCases; i++)
		{
			for (size_t j = 0; j < n; j++)
			{
				StackAllocator::GetInstance()->New<Sphere>();
			}
			StackAllocator::GetInstance()->CleanUp();
		}

		//Normal new/delete allocation.
		for (size_t i = 0; i < testCases; i++)
		{
			std::vector<Sphere*> sphereArray;
			for (size_t j = 0; j < n; j++)
			{
				sphereArray.push_back(StackAllocator::GetInstance()->New<Sphere>());
			}
			for (size_t j = n; j > 0; j--)
			{
				delete sphereArray[j - 1];
			}
		}
	}
}

void StackAllocatorTestThree()
{
	if (ImGui::Button("Test Case 3 - Random objects in a random order"))
	{
		const size_t n = 50000;
		const size_t testCases = 1000;

		//Randomize objects. Ints are between 1-3 inclusive.
		std::vector<int> randomInts;
		for (size_t i = 0; i < n; i++)
			randomInts.push_back(std::rand() % 3 + 1);

		//Stack allocator
		for (size_t i = 0; i < testCases; i++)
		{
			for (size_t j = 0; j < n; j++)
			{
				switch (randomInts[j])
				{
				case 1:
					StackAllocator::GetInstance()->New<Cube>();
					break;
				case 2:
					StackAllocator::GetInstance()->New<Sphere>();
					break;
				case 3:
					StackAllocator::GetInstance()->New<Pyramid>();
					break;
				default:
					break;
				}
			}
			StackAllocator::GetInstance()->CleanUp();
		}

		for (size_t i = 0; i < testCases; i++)
		{
			std::vector<Shape*> shapeArray;
			for (size_t j = 0; j < n; j++)
			{
				switch (randomInts[j])
				{
				case 1:
					shapeArray.push_back(StackAllocator::GetInstance()->New<Cube>());
					break;
				case 2:
					shapeArray.push_back(StackAllocator::GetInstance()->New<Sphere>());
					break;
				case 3:
					shapeArray.push_back(StackAllocator::GetInstance()->New<Pyramid>());
					break;
				default:
					break;
				}
			}
			for (size_t j = n; j > 0; j--)
			{
				delete shapeArray[j - 1];
			}
		}
	}
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
	//A lot of small objects. 400 000 Cubes
	//StackAllocatorTestOne();
	//A few large objects. 4000 Spheres
	//StackAllocatorTestTwo();
	//Random objects in a random order. 50 000 objects.
	//StackAllocatorTestThree();
	
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
			StackAllocator::GetInstance()->New<Cube>();

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
	m_TestResults.clear();
	uint64_t factor = 1;
	for (uint64_t i{ 0 }; i < 4; i++)
	{
		PoolAllocator<Cube> cubeAllocator("Cube Allocator", 1000 * factor);
		std::vector<Cube*> cubes;
		cubes.reserve(1000 * factor);
		for (uint64_t j = 0u; j < (1000 * factor); j++)
		{
			Cube* cube = nullptr;
			cubes.push_back(cube);
		}

		float allocationTimeSum = 0.0f;
		float deallocationTimeSum = 0.0f;
		std::string str1 = "Cube Pool allocation: Test 1 - " + std::to_string(1000 * factor) + " cubes";
		std::string str2 = "Cube Pool deallocation: Test 1 - " + std::to_string(1000 * factor) + " cubes";
		for (uint64_t k{ 0u }; k < 10; k++)
		{
			{
				PROFILE_TEST(str1.c_str());
				for (uint64_t l{ 0u }; l < (1000 * factor); l++)
				{
					cubes[l] = cubeAllocator.New();
				}
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
			{
				PROFILE_TEST(str2.c_str());
				for (uint64_t m{ 0u }; m < (1000 * factor); m++)
				{
					cubeAllocator.Delete(cubes[m]);
				}
			}
			deallocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}
		ProfileMetrics result = {};
		result.Name = str1.c_str();
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);
		result.Name = str2.c_str();
		result.Duration = deallocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		result = {};
		allocationTimeSum = 0.0f;
		deallocationTimeSum = 0.0f;

		str1 = "Cube New allocation: Test 1 - " + std::to_string(1000 * factor) + " cubes";
		str2 = "Cube New deallocation: Test 1 - " + std::to_string(1000 * factor) + " cubes";
		for (uint64_t n{ 0u }; n < 10u; n++)
		{
			{
				PROFILE_TEST(str1.c_str());
				for (uint64_t o{ 0u }; o < (1000 * factor); o++)
				{
					cubes[o] = DBG_NEW Cube;
				}
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
			{
				PROFILE_TEST(str2.c_str());
				for (uint64_t p{ 0u }; p < (1000 * factor); p++)
				{
					delete cubes[p];
				}
			}
			deallocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}
		result.Name = str1.c_str();
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);
		result.Name = str2.c_str();
		result.Duration = deallocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		factor *= 10;
	}
}

void Application::PerformPoolAllocatorTest2() noexcept
{
	m_TestResults.clear();
	uint64_t factor = 1;
	for (uint64_t i{ 0 }; i < 4; i++)
	{
		PoolAllocator<Pyramid> pyramidAllocator("Pyramid Allocator", 1000 * factor);
		std::vector<Pyramid*> pyramids;
		pyramids.reserve(1000 * factor);
		for (uint64_t j = 0u; j < (1000 * factor); j++)
		{
			Pyramid* pPyramid = nullptr;
			pyramids.push_back(pPyramid);
		}

		float allocationTimeSum = 0.0f;
		float deallocationTimeSum = 0.0f;
		std::string str1 = "Pyramid Pool allocation: Test 2 - " + std::to_string(1000 * factor) + " pyramids";
		std::string str2 = "Pyramid Pool deallocation: Test 2 - " + std::to_string(1000 * factor) + " pyramids";
		for (uint64_t k{ 0u }; k < 10u; k++)
		{
			{
				PROFILE_TEST(str1.c_str());
				for (uint64_t l{ 0u }; l < (1000 * factor); l++)
				{
					pyramids[l] = pyramidAllocator.New();
				}
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
			{
				PROFILE_TEST(str2.c_str());
				for (uint64_t m{ 0u }; m < (1000 * factor); m++)
				{
					pyramidAllocator.Delete(pyramids[m]);
				}
			}
			deallocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}
		ProfileMetrics result = {};
		result.Name = str1.c_str();
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);
		result.Name = str2.c_str();
		result.Duration = deallocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		result = {};
		allocationTimeSum = 0.0f;
		deallocationTimeSum = 0.0f;

		str1 = "Pyramid New allocation: Test 2 - " + std::to_string(1000 * factor) + " pyramids";
		str2 = "Pyramid New deallocation: Test 2 - " + std::to_string(1000 * factor) + " pyramids";
		for (uint64_t n{ 0u }; n < 10u; n++)
		{
			{
				PROFILE_TEST(str1.c_str());
				for (uint64_t o{ 0u }; o < (1000 * factor); o++)
				{
					pyramids[o] = DBG_NEW Pyramid;
				}
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
			{
				PROFILE_TEST(str2.c_str());
				for (uint64_t p{ 0u }; p < (1000 * factor); p++)
				{
					delete pyramids[p];
				}
			}
			deallocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}
		result.Name = str1.c_str();
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);
		result.Name = str2.c_str();
		result.Duration = deallocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		factor *= 10;
	}
}

void Application::PerformPoolAllocatorTest3() noexcept
{
	m_TestResults.clear();
	uint64_t factor = 1;
	for (uint64_t i{ 0 }; i < 4; i++)
	{
		PoolAllocator<Sphere> sphereAllocator("Sphere Allocator", 1000 * factor);
		std::vector<Sphere*> spheres;
		spheres.reserve(1000 * factor);
		for (uint64_t j = 0u; j < (1000 * factor); j++)
		{
			Sphere* sphere = nullptr;
			spheres.push_back(sphere);
		}

		float allocationTimeSum = 0.0f;
		float deallocationTimeSum = 0.0f;
		std::string str1 = "Sphere Pool allocation: Test 3 - " + std::to_string(1000 * factor) + " spheres";
		std::string str2 = "Sphere Pool deallocation: Test 3 - " + std::to_string(1000 * factor) + " spheres";
		for (uint64_t k{ 0u }; k < 10u; k++)
		{
			{
				PROFILE_TEST(str1.c_str());
				for (uint64_t l{ 0u }; l < (1000 * factor); l++)
				{
					spheres[l] = sphereAllocator.New();
				}
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
			{
				PROFILE_TEST(str2.c_str());
				for (uint64_t m{ 0u }; m < (1000 * factor); m++)
				{
					sphereAllocator.Delete(spheres[m]);
				}
			}
			deallocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}
		ProfileMetrics result = {};
		result.Name = str1.c_str();
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);
		result.Name = str2.c_str();
		result.Duration = deallocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		result = {};
		allocationTimeSum = 0.0f;
		deallocationTimeSum = 0.0f;

		str1 = "Cube New allocation: Test 3 - " + std::to_string(1000 * factor) + " spheres";
		str2 = "Cube New deallocation: Test 3 - " + std::to_string(1000 * factor) + " spheres";
		for (uint64_t n{ 0u }; n < 10u; n++)
		{
			{
				PROFILE_TEST(str1.c_str());
				for (uint64_t o{ 0u }; o < (1000 * factor); o++)
				{
					spheres[o] = DBG_NEW Sphere;
				}
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
			{
				PROFILE_TEST(str2.c_str());
				for (uint64_t p{ 0u }; p < (1000 * factor); p++)
				{
					delete spheres[p];
				}
			}
			deallocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}
		result.Name = str1.c_str();
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);
		result.Name = str2.c_str();
		result.Duration = deallocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		factor *= 10;
	}
}
