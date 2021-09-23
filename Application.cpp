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
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;
static uint32_t index = 0u;
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
	}
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
	

		UI::Begin();
		// Windows not part of the dock space goes here:


		//...And ends here.
		UI::BeginDockSpace();
		//Windows part of the dock space goes here:

		ImGui::Begin("Allocator settings");
		static int nrOfCubesToAllocate = 0;
		ImGui::InputInt("Nr of cubes to allocate", &nrOfCubesToAllocate, 1000);
		static bool usePoolAllocator = false;
		static bool useNewAllocator = false;
		static bool allocateInChunks = false;
		static bool deallocateEveryFrame = true;

		ImGui::Checkbox("Pool Allocator", &usePoolAllocator);
		ImGui::Checkbox("New Allocator", &useNewAllocator);
		ImGui::Checkbox("Deallocate every frame", &deallocateEveryFrame);
		
		if (deallocateEveryFrame && index > 0u)
		{
			ResetAll();
		}


		ImGui::End();
		{
			//Scope could be used for profiling total time.
			if (usePoolAllocator)
			{
				if (deallocateEveryFrame)
				{
					PoolAllocateCubes(m_pCubesPool, nrOfCubesToAllocate);
					PoolDeallocateCubes(m_pCubesPool, nrOfCubesToAllocate);
				}
				else
				{
					index = PoolAllocateCubes(m_pCubesPool, nrOfCubesToAllocate, index);
				}
			}
		}
		{
			//Scope could be used for profiling total time.
			if (useNewAllocator)
			{
				NewAllocateCubes(m_pCubesNew, nrOfCubesToAllocate);
				NewDeallocateCubes(m_pCubesNew, nrOfCubesToAllocate);
			}
		}

		ImGui::Begin("Progress bar");
		// Animate a simple progress bar
		static float progress = 0.0f;
		
		progress = static_cast<float>(m_CubeAllocator.GetUsage() / static_cast<float>(m_CubeAllocator.GetCapacity()));
		progress = 1.0f - progress;

		ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("Progress Bar");

		float progress_saturated = std::clamp(progress, 0.0f, 1.0f);
		char buf[64];
#pragma warning(disable:4996)
		sprintf(buf, "%d/%d", (int)((m_CubeAllocator.GetEntityCapacity() - m_CubeAllocator.GetEntityUsage())), (int)(m_CubeAllocator.GetEntityCapacity()));
		ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), buf);
		ImGui::End();


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
/*Function ONLY allocates, deallocation is done in another function
  and requires the user to know what Cube* in the array has been allocated.
  For testing purposes.*/
void Application::PoolAllocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToAlloc) noexcept
{
	std::string str = __FUNCTION__ " (" + std::to_string(nrOfCubesToAlloc) + ")";
	PROFILE_SCOPE(str);

	for (uint32_t i{ 0u }; i < nrOfCubesToAlloc; ++i)
	{
		pCubes[i] = m_CubeAllocator.New();
	}
}

uint32_t Application::PoolAllocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToAlloc, const uint32_t startIndex) noexcept
{
	std::string str = __FUNCTION__ " (" + std::to_string(nrOfCubesToAlloc) + ")";
	uint32_t i{ startIndex };
	uint32_t stop = i + nrOfCubesToAlloc;
	stop = stop <= m_CubeAllocator.GetEntityCapacity() ? stop : m_CubeAllocator.GetEntityCapacity();
	{
		PROFILE_SCOPE(str);
		for (i; i < stop; ++i)
		{
			pCubes[i] = m_CubeAllocator.New();
		}
	}

	return i;
}

void Application::PoolDeallocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToDealloc) noexcept
{
	std::string str = __FUNCTION__ " (" + std::to_string(nrOfCubesToDealloc) + ")";
	PROFILE_SCOPE(str);
	for (uint32_t i{ 0u }; i < nrOfCubesToDealloc; ++i)
	{
		m_CubeAllocator.Delete(pCubes[i]);
	}
}

void Application::NewAllocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToAlloc) noexcept
{
	std::string str = __FUNCTION__ " (" + std::to_string(nrOfCubesToAlloc) + ")";
	PROFILE_SCOPE(str);
	for (uint32_t i{ 0u }; i < nrOfCubesToAlloc; ++i)
	{
		pCubes[i] = new Cube;
	}
}

void Application::NewDeallocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToDealloc) noexcept
{
	std::string str = __FUNCTION__ " (" + std::to_string(nrOfCubesToDealloc) + ")";
	PROFILE_SCOPE(str);
	for (uint32_t i{ 0u }; i < nrOfCubesToDealloc; ++i)
	{
		delete pCubes[i];
	}
}

void Application::ResetAll() noexcept
{
	for (uint32_t i{ 0u }; i < index; ++i)
	{
		m_CubeAllocator.Delete(m_pCubesPool[i]);
	}
	index = 0;
}
