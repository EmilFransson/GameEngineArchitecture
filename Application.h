#pragma once
#include "Window.h"
#include "UI.h"
#include "Profiler.h"
#include "PoolAllocator.h"
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
	void PoolAllocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToAlloc) noexcept;
	uint32_t PoolAllocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToAlloc, const uint32_t startIndex) noexcept;
	void PoolDeallocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToDealloc) noexcept;
	void NewAllocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToAlloc) noexcept;
	void NewDeallocateCubes(std::vector<Cube*>& pCubes, const uint32_t nrOfCubesToDealloc) noexcept;
	void ResetAll() noexcept;

private:
	std::vector<ProfileMetrics> m_ProfileMetrics;
	bool m_Running;
	std::unique_ptr<UI> m_pImGui;
	PoolAllocator<Cube> m_CubeAllocator;
	std::vector<Cube*> m_pCubesPool;
	std::vector<Cube*> m_pCubesNew;
};