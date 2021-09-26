#pragma once
#include "Window.h"
#include "UI.h"
#include "Profiler.h"
#include "PoolAllocator.h"
#include "BuddyAllocator.hpp"
#include "Cube.h"

class Application
{
public:
	Application() noexcept;
	~Application() noexcept = default;
	void Run() noexcept;
private:
	void DisplayProfilingResults() noexcept;
	void AllocateCubes(uint32_t nrOfCubes) noexcept;
private:
	std::vector<ProfileMetrics> m_ProfileMetrics;
	bool m_Running;
	std::unique_ptr<UI> m_pImGui;
	PoolAllocator<Cube> m_CubeAllocator;
	BuddyAllocator m_buddyAllocator;
};