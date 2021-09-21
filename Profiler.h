#pragma once

struct ProfileMetrics
{
	std::string Name;
	float Duration;
};

template<class lambdaFunction>
class Profiler
{
public:
	Profiler(const std::string functionName, const lambdaFunction&& func) noexcept
		: m_FunctionName{ std::move(functionName) }, m_LambdaFunction{std::move(func)}
	{
		m_StartPoint = std::chrono::high_resolution_clock::now();
	}

	~Profiler() noexcept
	{
		std::chrono::time_point<std::chrono::steady_clock> endPoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartPoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();

		float durationInMilliseconds = (end - start) * 0.001f;
		m_LambdaFunction({ m_FunctionName, durationInMilliseconds });
	}
private:
	std::string m_FunctionName;
	std::chrono::time_point<std::chrono::steady_clock> m_StartPoint;
	const lambdaFunction m_LambdaFunction;
};
