#pragma once
#include "Window.h"
#include "UI.h"
#include "Profiler.h"
#include "PoolAllocator.h"
#include "BuddyAllocator.hpp"
#include "Cube.h"
#include "ObjectClasses.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define PROFILE_FUNC Profiler TOKENPASTE2(profiler, __LINE__) (__FUNCTION__, [&](ProfileMetrics profileMetrics) {m_ProfileMetrics.push_back(std::move(profileMetrics));})
#define PROFILE_SCOPE(scopeName) Profiler TOKENPASTE2(profiler, __LINE__) (scopeName, [&](ProfileMetrics profileMetrics) {m_ProfileMetrics.push_back(std::move(profileMetrics));})
#define PROFILE_TEST(scopeName) Profiler TOKENPASTE2(profiler, __LINE__) (scopeName, [&](ProfileMetrics profileMetrics) {m_RepeatedTests.push_back(std::move(profileMetrics)); })

class Application
{
public:
	Application() noexcept;
	~Application() noexcept = default;
	void Run() noexcept;
private:
	void DisplayProfilingResults() noexcept;
	template<typename T>
	void PoolAllocateObjects(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects, const uint64_t nrOfObjectsToAlloc) noexcept;
	template<typename T>
	void PoolDeallocateObjects(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects, uint64_t nrOfObjectsToDealloc) noexcept;
	template<typename T>
	void NewAllocateObjects(std::vector<T*>& objects, const uint64_t nrOfObjectsToAlloc) noexcept;
	template<typename T>
	void NewDeallocateObjects(std::vector<T*>& objects, const uint64_t nrOfObjectsToDealloc) noexcept;
	void BuddyAllocate() noexcept;
	void BuddyDeallocate() noexcept;
	template<typename T>
	void ResetPoolAllocator(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects) noexcept;
	template<typename T>
	void RenderPoolAllocatorSettingsPanel(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects) noexcept;
	void RenderNewAllocatorSettingsPanel() noexcept;
	void RenderBuddyAllocatorSettingsPanel() noexcept;
	template<typename T>
	void RenderPoolAllocatorProgressBar(PoolAllocator<T>& poolAllocator) noexcept;

	void RenderBuddyProgressBar() noexcept;

	void StackAllocateObjects() noexcept;
	void StackAllocatorTestOne();
	void StackAllocatorTestTwo();
	void StackAllocatorTestThree();
	void RenderStackAllocatorProgressBar() noexcept;
	void PerformPoolAllocatorTest1() noexcept;
	void PerformPoolAllocatorTest2() noexcept;
	void PerformPoolAllocatorTest3() noexcept;
private:
	std::vector<ProfileMetrics> m_ProfileMetrics;
	std::vector<ProfileMetrics> m_RepeatedTests;
	std::vector<ProfileMetrics> m_TestResults;
	bool m_Running;
	std::unique_ptr<UI> m_pImGui;
	BuddyAllocator m_buddyAllocator;
	PoolAllocator<Cube> m_CubeAllocator = PoolAllocator<Cube>("Cube allocator");
	std::vector<Cube*> m_pCubesPool;
	std::vector<Cube*> m_pCubesNew;
	static int s_NrOfCubesToPoolAllocate;
	static bool s_DeallocateEveryFrame;

	// Buddy Allocator Settings
	std::vector<void*> m_buddyAllocations;
	int m_buddyAllocationSize;
	int m_buddyAllocationCount;
	bool m_buddyEnabled;
	bool m_buddyDealloc;
	bool m_buddyAllocatorFull;

	size_t m_buddyUnusedMemory = 0;
};

template<typename T>
void Application::PoolAllocateObjects(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects, const uint64_t nrOfObjectsToAlloc) noexcept
{
	if (poolAllocator.GetEntityUsage() == poolAllocator.GetEntityCapacity())
	{
		//PoolAllocator is full.
		return;
	}
	uint64_t allocationLimit = poolAllocator.GetEntityUsage() + nrOfObjectsToAlloc;
	if (allocationLimit > poolAllocator.GetEntityCapacity())
	{
		return;
	}
	std::string str = __FUNCTION__;
	str.append(" '").append(poolAllocator.GetTag()).append("' (").append(std::to_string(nrOfObjectsToAlloc).c_str()).append(")");
	PROFILE_SCOPE(str);
	for (uint64_t i{ poolAllocator.GetEntityUsage() }; i < allocationLimit; i++)
	{
		objects[i] = poolAllocator.New();
	}
}

template<typename T>
void Application::PoolDeallocateObjects(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects, uint64_t nrOfObjectsToDealloc) noexcept
{
	int start = static_cast<int>(poolAllocator.GetEntityUsage() - 1);
	if (start < 0)
	{
		return;
	}
	if (nrOfObjectsToDealloc > poolAllocator.GetEntityUsage())
	{
		return;
	}
	std::string str = __FUNCTION__;
	str.append(" '").append(poolAllocator.GetTag()).append("' (").append(std::to_string(nrOfObjectsToDealloc).c_str()).append(")");
	int end = static_cast<int>(start - nrOfObjectsToDealloc);
	PROFILE_SCOPE(str);
	for (int i{ start }; i > end; i--)
	{
		poolAllocator.Delete(objects[i]);
	}
}

template<typename T>
void Application::NewAllocateObjects(std::vector<T*>& objects, const uint64_t nrOfObjectsToAlloc) noexcept
{
	std::string str = __FUNCTION__ " (" + std::to_string(nrOfObjectsToAlloc) + ")";
	PROFILE_SCOPE(str);
	for (uint64_t i{ 0u }; i < nrOfObjectsToAlloc; ++i)
	{
		objects[i] = DBG_NEW Cube;
	}
}

template<typename T>
void Application::NewDeallocateObjects(std::vector<T*>& objects, const uint64_t nrOfObjectsToDealloc) noexcept
{
	std::string str = __FUNCTION__ " (" + std::to_string(nrOfObjectsToDealloc) + ")";
	PROFILE_SCOPE(str);
	for (uint64_t i{ 0u }; i < nrOfObjectsToDealloc; ++i)
	{
		delete objects[i];
	}
}

template<typename T>
void Application::ResetPoolAllocator(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects) noexcept
{
	uint64_t cubesToDeallocate = poolAllocator.GetEntityUsage();
	for (uint64_t i{ 0u }; i < cubesToDeallocate; ++i)
	{
		poolAllocator.Delete(objects[i]);
	}
}

template<typename T>
void Application::RenderPoolAllocatorSettingsPanel(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects) noexcept
{
	std::string settings = "PoolAllocator Settings (";
	settings.append(poolAllocator.GetTag()).append(")");
	ImGui::Begin(settings.c_str());
	static bool pressed = false;
	if (ImGui::Checkbox("Enable", &pressed))
	{
		poolAllocator.ToggleEnabled();
		if (!poolAllocator.IsEnabled())
		{
			ResetPoolAllocator<Cube>(poolAllocator, objects);
		}
	}
	if (poolAllocator.IsEnabled())
	{
		static bool pressedAlloc = true;
		static bool allocDeallocSame = false;
		if (ImGui::Checkbox("Alloc & Dealloc same amount", &allocDeallocSame))
		{
			poolAllocator.ToggleAllocDeallocSameAmount();
		}
		if (ImGui::Checkbox("Allocate on frame", &pressedAlloc))
		{
			poolAllocator.ToggleAllocateOnFrame();
		}
		static bool pressedDealloc = true;
		if (ImGui::Checkbox("Deallocate on frame", &pressedDealloc))
		{
			poolAllocator.ToggleDeallocateOnFrame();
		}
		if (!poolAllocator.IsAllocatingAndDeallocatingSameAmount())
		{
			int nrOfObjectsToAlloc = static_cast<int>(poolAllocator.GetNrOfEntitiesAllocatedEveryFrame());
			if (ImGui::InputInt("#Objects to allocate.", &nrOfObjectsToAlloc, 1000))
			{
				if (nrOfObjectsToAlloc < 0)
				{
					nrOfObjectsToAlloc = 0;
				}
				poolAllocator.SetNrOfEntitiesAllocatedEveryFrame(nrOfObjectsToAlloc);
			}
			int nrOfObjectsToDealloc = static_cast<int>(poolAllocator.GetNrOfEntitiesDeallocatedEveryFrame());
			if (ImGui::InputInt("#Objects to deallocate.", &nrOfObjectsToDealloc, 1000))
			{
				if (nrOfObjectsToDealloc < 0)
				{
					nrOfObjectsToDealloc = 0;
				}
				poolAllocator.SetNrOfEntitiesDeallocatedEveryFrame(nrOfObjectsToDealloc);
			}
		}
		else
		{
			static int nrOfObjectsToAllocAndDealloc = 0;
			if (ImGui::InputInt("#Objects to Allocate/Deallocate.", &nrOfObjectsToAllocAndDealloc, 1000))
			{
				if (nrOfObjectsToAllocAndDealloc < 0)
				{
					nrOfObjectsToAllocAndDealloc = 0;
				}
				poolAllocator.SetNrOfEntitiesAllocatedEveryFrame(nrOfObjectsToAllocAndDealloc);
				poolAllocator.SetNrOfEntitiesDeallocatedEveryFrame(nrOfObjectsToAllocAndDealloc);
			}
		}
		if (ImGui::Button("Test 1 - 400 000 cubes"))
		{
			PerformPoolAllocatorTest1();
		}
		if (ImGui::Button("Test 2 - Placeholder"))
		{
			PerformPoolAllocatorTest2();
		}
		if (ImGui::Button("Test 3 - Placeholder"))
		{
			PerformPoolAllocatorTest3();
		}

		RenderPoolAllocatorProgressBar<T>(poolAllocator);
	}
	ImGui::End();
}

template<typename T>
void Application::RenderPoolAllocatorProgressBar(PoolAllocator<T>& poolAllocator) noexcept
{
	ImGui::Begin("Pool Allocator memory usage");
	ImGui::Text("Tag:");
	ImGui::SameLine();
	ImGui::Text(poolAllocator.GetTag());
	static float progress = 0.0f;

	progress = static_cast<float>(poolAllocator.GetUsage() / static_cast<float>(poolAllocator.GetCapacity()));
	progress = 1.0f - progress;

	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Bytes free.");

	char buf[64];
#pragma warning(disable:4996)
	sprintf(buf, "%d/%d", (int)((poolAllocator.GetEntityCapacity() - poolAllocator.GetEntityUsage())), (int)(poolAllocator.GetEntityCapacity()));
	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f), buf);
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Entity chunks available.");
	ImGui::End();
}
