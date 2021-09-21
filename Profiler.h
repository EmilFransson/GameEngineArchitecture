#pragma once
class Profiler
{
public:
	Profiler(const char* functionName) noexcept;
	~Profiler() noexcept;
private:
	const char* m_FunctionName;
	std::chrono::time_point<std::chrono::steady_clock> m_StartPoint;
};

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define PROFILE_FUNC Profiler TOKENPASTE2(profiler, __LINE__) (__FUNCTION__)
#define PROFILE_SCOPE(scopeName) Profiler TOKENPASTE2(profiler, __LINE__) (scopeName)
