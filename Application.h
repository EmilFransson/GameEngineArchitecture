#pragma once
#include "Window.h"
class Application
{
public:
	Application() noexcept;
	~Application() noexcept = default;
	void Run() noexcept;
private:
	bool m_Running;
};