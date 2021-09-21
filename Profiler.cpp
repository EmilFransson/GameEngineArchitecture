#include "pch.h"
#include "Profiler.h"

Profiler::Profiler(const char* functionName) noexcept
	: m_FunctionName{functionName}
{
	m_StartPoint = std::chrono::high_resolution_clock::now();
}

Profiler::~Profiler() noexcept
{
	std::chrono::time_point<std::chrono::steady_clock> endPoint = std::chrono::high_resolution_clock::now();

	long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartPoint).time_since_epoch().count();
	long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();

	float durationInMilliseconds = (end - start) * 0.001f;
	std::cout << m_FunctionName << ": " << durationInMilliseconds << "ms\n";
}
