#pragma once
#include "Window.h"
#include "UI.h"
#include "Profiler.h"
#include "PoolAllocator.h"
#include "Cube.h"

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define PROFILE_FUNC Profiler TOKENPASTE2(profiler, __LINE__) (__FUNCTION__, [&](ProfileMetrics profileMetrics) {m_ProfileMetrics.push_back(std::move(profileMetrics));})
#define PROFILE_SCOPE(scopeName) Profiler TOKENPASTE2(profiler, __LINE__) (scopeName, [&](ProfileMetrics profileMetrics) {m_ProfileMetrics.push_back(std::move(profileMetrics));})

class Application
{
public:
	Application() noexcept;
	~Application() noexcept = default;
	void Run() noexcept;
private:
	void DisplayProfilingResults() noexcept;
	void AllocateCubes(uint32_t nrOfCubes) noexcept;
	template<typename T>
	void PoolAllocateObjects(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects, const uint64_t nrOfObjectsToAlloc) noexcept;
	template<typename T>
	void PoolDeallocateObjects(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects, const uint64_t nrOfObjectsToDealloc) noexcept;
	template<typename T>
	void NewAllocateObjects(std::vector<T*>& objects, const uint64_t nrOfObjectsToAlloc) noexcept;
	template<typename T>
	void NewDeallocateObjects(std::vector<T*>& objects, const uint64_t nrOfObjectsToDealloc) noexcept;
	template<typename T>
	void ResetPoolAllocator(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects) noexcept;
	template<typename T>
	void RenderPoolAllocatorSettingsPanel(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects) noexcept;
	void RenderNewAllocatorSettingsPanel() noexcept;

	void RenderStackAllocatorSettingsPanel() noexcept;
	void StackAllocateObjects() noexcept;
private:
	std::vector<ProfileMetrics> m_ProfileMetrics;
	bool m_Running;
	std::unique_ptr<UI> m_pImGui;
	PoolAllocator<Cube> m_CubeAllocator = PoolAllocator<Cube>("Cube allocator");
	std::vector<Cube*> m_pCubesPool;
	std::vector<Cube*> m_pCubesNew;
	static int s_NrOfCubesToPoolAllocate;
	static bool s_DeallocateEveryFrame;
};

template<typename T>
void Application::PoolAllocateObjects(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects, const uint64_t nrOfObjectsToAlloc) noexcept
{
	if (poolAllocator.GetEntityUsage() == poolAllocator.GetEntityCapacity())
	{
		//PoolAllocator is full.
		return;
	}
	std::string str = __FUNCTION__;
	str.append(" '");
	str.append(poolAllocator.GetTag());
	str.append("' (");
	str.append(std::to_string(nrOfObjectsToAlloc).c_str());
	str.append(")");
	uint64_t allocationLimit = poolAllocator.GetEntityUsage() + nrOfObjectsToAlloc;
	if (allocationLimit > poolAllocator.GetEntityCapacity())
	{
		allocationLimit = poolAllocator.GetEntityCapacity();
	}
	PROFILE_SCOPE(str);
	for (uint64_t i{ poolAllocator.GetEntityUsage() }; i < allocationLimit; i++)
	{
		objects[i] = poolAllocator.New();
	}
}

template<typename T>
void Application::PoolDeallocateObjects(PoolAllocator<T>& poolAllocator, std::vector<T*>& objects, const uint64_t nrOfObjectsToDealloc) noexcept
{
	std::string str = __FUNCTION__;
	str.append(" '");
	str.append(poolAllocator.GetTag());
	str.append("' (");
	str.append(std::to_string(nrOfObjectsToDealloc).c_str());
	str.append(")");
	PROFILE_SCOPE(str);
	for (uint64_t i{ 0u }; i < nrOfObjectsToDealloc; ++i)
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
	settings.append(poolAllocator.GetTag());
	settings.append(")");
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
	if (ImGui::Checkbox("Deallocate every frame", &s_DeallocateEveryFrame))
	{
		if (s_DeallocateEveryFrame)
			ResetPoolAllocator<Cube>(poolAllocator, objects);
	}
	if (ImGui::InputInt("#Cubes to allocate.", &s_NrOfCubesToPoolAllocate, 1000))
	{
		if (s_NrOfCubesToPoolAllocate < 0)
			s_NrOfCubesToPoolAllocate = 0;
	}
	ImGui::End();
}
