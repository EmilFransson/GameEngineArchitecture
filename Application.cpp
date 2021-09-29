#include "pch.h"
#include "Application.h"
#include "System.h"
#include "RenderCommand.h"
#include "StackAllocator.h"

#define MEGA 1000000ll
#define GIGA 1000000000ll
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
	StackAllocator::CreateAllocator(GIGA * 5ll);
	while (m_Running)
	{
		static const FLOAT color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
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
		RenderBuddyAllocatorSettingsPanel();

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
		{
			if(m_buddyEnabled)
				BuddyAllocate();
			if (m_buddyDealloc)
				BuddyDeallocate();
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

void Application::StackAllocatorTestOne()
{
	if (ImGui::Button("Test Case 1 - Many small objects"))
	{
		const size_t n = 400000;
		const size_t testCases = 10;

		float allocationTimeSum = 0.0f;
		//Stack Allocator
		for (size_t i = 0; i < testCases; i++)
		{
			{
				PROFILE_TEST("Cube Stack allocation & deallocation: Test 1 - 400 000 cubes");
				for (size_t j = 0; j < n; j++)
				{
					StackAllocator::GetInstance()->New<Cube>();
				}
				StackAllocator::GetInstance()->CleanUp();
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}

		ProfileMetrics result = {};
		result.Name = "Cube Stack allocation: Test 1 - 400 000 cubes";
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		result = {};
		allocationTimeSum = 0.0f;

		//Normal new/delete allocation.
		for (size_t i = 0; i < testCases; i++)
		{
			{
				PROFILE_TEST("New allocation & deallocation: Test 1 - 400 000 cubes");
				std::vector<Cube*> cubeArray;
				for (size_t j = 0; j < n; j++)
				{
					cubeArray.push_back(new Cube());
				}
				for (size_t j = n; j > 0; j--)
				{
					delete cubeArray[j - 1];
				}
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}

		result.Name = "Cube New allocation: Test 1 - 400 000 cubes";
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
	}

	// BuddyAllocator
	str = "Application::AllocateCubes (" + std::to_string(nrOfCubes) + ") - Buddy Allocator";
	Cube** cubes = new Cube * [10000];
	{
		PROFILE_SCOPE(str);
		constexpr auto lol = sizeof(Cube);
		cubes[0] = (Cube*) m_buddyAllocator.alloc(sizeof(Cube) * nrOfCubes);
		m_buddyAllocator.free(cubes[0], sizeof(Cube) * nrOfCubes);
	}
	delete[] cubes;
}

void Application::StackAllocatorTestTwo()
{
	if (ImGui::Button("Test Case 2 - Few large objects"))
	{
		const size_t n = 40000;
		const size_t testCases = 10;

		float allocationTimeSum = 0.0f;
		//Stack Allocator
		for (size_t i = 0; i < testCases; i++)
		{
			{
				PROFILE_TEST("Sphere Stack allocation & deallocation: Test 2 - 40 000 Spheres");
				for (size_t j = 0; j < n; j++)
				{
					StackAllocator::GetInstance()->New<Sphere>();
				}
				StackAllocator::GetInstance()->CleanUp();
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}

		ProfileMetrics result = {};
		result.Name = "Sphere Stack allocation: Test 2 - 40 000 Spheres";
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		result = {};
		allocationTimeSum = 0.0f;

		//Normal new/delete allocation.
		for (size_t i = 0; i < testCases; i++)
		{
			{
				PROFILE_TEST("Sphere New allocation & deallocation: Test 2 - 40 000 Spheres");
				std::vector<Sphere*> sphereArray;
				for (size_t j = 0; j < n; j++)
				{
					sphereArray.push_back(new Sphere());
				}
				for (size_t j = n; j > 0; j--)
				{
					delete sphereArray[j - 1];
				}
			}
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}

		result.Name = "Sphere New allocation: Test 2 - 40 000 Spheres";
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
	}
}

void Application::StackAllocatorTestThree()
{
	if (ImGui::Button("Test Case 3 - Random objects in a random order"))
	{
		const size_t n = 500000;
		const size_t testCases = 100; //More testcases for random

		//Randomize objects. Ints are between 1-3 inclusive.
		std::vector<int> randomInts;
		std::srand(std::time(0));
		for (size_t i = 0; i < n; i++)
			randomInts.push_back(std::rand() % 3 + 1);

		float allocationTimeSum = 0.0f;
		//Stack allocator
		for (size_t i = 0; i < testCases; i++)
		{
			{
				PROFILE_TEST("Random Stack allocation & deallocation: Test 3 - 500 000 objects");
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
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}

		ProfileMetrics result = {};
		result.Name = "Random Stack allocation: Test 3 - 500 000 objects";
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
		result = {};
		allocationTimeSum = 0.0f;

		//Normal new/delete
		for (size_t i = 0; i < testCases; i++)
		{
			{
				PROFILE_TEST("Random New allocation & deallocation: Test 3 - 500 000 objects");
				std::vector<Shape*> shapeArray;
				for (size_t j = 0; j < n; j++)
				{
					switch (randomInts[j])
					{
					case 1:
						shapeArray.push_back(new Cube());
						break;
					case 2:
						shapeArray.push_back(new Sphere());
						break;
					case 3:
						shapeArray.push_back(new Pyramid());
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
			allocationTimeSum += m_RepeatedTests[m_RepeatedTests.size() - 1].Duration;
		}
		result.Name = "Random New allocation: Test 3 - 500 000 objects";
		result.Duration = allocationTimeSum / 10.0f;
		m_TestResults.push_back(result);

		m_RepeatedTests.clear();
	}
}

void Application::RenderBuddyAllocatorSettingsPanel() noexcept
{
	ImGui::Begin("Buddy Allocator");
	ImGui::Checkbox("Enable", &m_buddyEnabled);
	if (m_buddyEnabled)
	{
		ImGui::InputInt("Size of allocations", &m_buddyAllocationSize, 512);
		ImGui::InputInt("Number of allocations", &m_buddyAllocationCount, 1000);
		ImGui::Checkbox("Deallocate", &m_buddyDealloc);
		if (ImGui::Button("Reset"))
		{
			m_buddyAllocator.reset();
		}
		RenderBuddyProgressBar();
	}

	if (m_buddyAllocationSize < 0)
		m_buddyAllocationSize = 0;
	if (m_buddyAllocationCount < 0)
		m_buddyAllocationCount = 0;

	ImGui::End();
}

void Application::BuddyAllocate() noexcept
{
	if (m_buddyAllocations.size() < m_buddyAllocationCount)
	{
		m_buddyAllocations.clear();
		m_buddyAllocations.resize(m_buddyAllocationCount);
	}
	if (m_buddyDealloc) { m_buddyAllocator.reset(); }

	std::string str = __FUNCTION__ " (" + std::to_string(m_buddyAllocationCount) + ")";
	{
		PROFILE_SCOPE(str);
		m_buddyAllocatorFull = false;
		for (auto i = 0u; i < m_buddyAllocationCount; ++i)
		{
			m_buddyAllocations[i] = m_buddyAllocator.alloc(m_buddyAllocationSize);

			if (!m_buddyAllocations[i])
			{
				m_buddyAllocatorFull = true;
				break;
			}
		}
	}
	m_buddyUnusedMemory = m_buddyAllocator.getUnusedMemory();
}

void Application::BuddyDeallocate() noexcept
{
	std::string str = __FUNCTION__ " (" + std::to_string(m_buddyAllocationCount) + ")";
	PROFILE_SCOPE(str);
	std::for_each(m_buddyAllocations.begin(), m_buddyAllocations.end(), [this](void* ptr) { m_buddyAllocator.free(ptr, m_buddyAllocationSize); });
}

void Application::RenderBuddyProgressBar() noexcept
{
	ImGui::Text("Memory: ");
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	float frac = (float)m_buddyUnusedMemory / (float)MAX_BLOCK;
	ImGui::ProgressBar(frac, ImVec2(0, 0));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("%d / %d", m_buddyUnusedMemory, MAX_BLOCK);
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
	StackAllocatorTestOne();
	//A few large objects. 4000 Spheres
	StackAllocatorTestTwo();
	//Random objects in a random order. 50 000 objects.
	StackAllocatorTestThree();

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
	PoolAllocator<Cube> cubeAllocator("Cube Allocator", 400000u);
	std::vector<Cube*> cubes;
	cubes.reserve(400000);
	for (uint64_t i = 0u; i < 400000; i++)
	{
		Cube* cube = nullptr;
		cubes.push_back(cube);
	}

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

void Application::PerformPoolAllocatorTest2() noexcept
{
}

void Application::PerformPoolAllocatorTest3() noexcept
{
}
