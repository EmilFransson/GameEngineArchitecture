#include "pch.h"
#include "Window.h"
#include "Utility.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "ImGui\imgui.h"
#include "Profiler.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const Window Window::m_sInstance;
std::wstring Window::m_Title = L"?";
std::wstring Window::m_ClassName = L"GameEngineArchitectureWindowClass";
uint32_t Window::m_Width = 0u;
uint32_t Window::m_Height = 0u;
HWND Window::m_WindowHandle;
MSG Window::m_WindowMessage = { 0 };
bool Window::m_Running = true;

const Window& Window::Get() noexcept
{
	return m_sInstance;
}

Window::~Window() noexcept
{
	ShutDown();
}

void Window::Initialize(const std::wstring& windowTitle, const uint32_t width, const uint32_t height)
{
	WNDCLASSEX windowClass = { 0 };						//[https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexa]
	windowClass.cbSize = sizeof(WNDCLASSEX);			//Size in bytes of structure.
	windowClass.style = CS_OWNDC;						//Every window get its own device context.
	windowClass.lpfnWndProc = HandleMessages;			//Long pointer to function handling the window messages (setup in this case).
	windowClass.cbClsExtra = 0;							//Extra bytes allocated to the window class structure.
	windowClass.cbWndExtra = 0;							//Extra bytes allocated following the window instance.
	windowClass.hInstance = GetModuleHandle(nullptr); 	//Handle to the instance that contains the window procedure for class.
	windowClass.hIcon = nullptr;						//Handle to the class icon.
	windowClass.hCursor = nullptr;						//Handle to the class cursor.
	windowClass.hbrBackground = nullptr;				//Handle to the class background brush.
	windowClass.lpszMenuName = nullptr;					//Pointer to null-terminated character string specifying resource name of class menu.
	windowClass.lpszClassName = m_ClassName.c_str();	//Window class name
	RegisterClassEx(&windowClass);

	/*Calculate the rectangle equivalent to the client region of the window based
	  on the window styles included for the window.*/
	RECT windowRect = { 0 };
	windowRect.left = 0;
	windowRect.right = width + windowRect.left;
	windowRect.top = 0;
	windowRect.bottom = windowRect.top + height;
	AdjustWindowRect(&windowRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	//Calculate dimension-agnostic start position for window:
	int windowStartPositionX = static_cast<int>((GetSystemMetrics(SM_CXSCREEN) *
		(((GetSystemMetrics(SM_CXSCREEN) - width)
			/ (float)GetSystemMetrics(SM_CXSCREEN)) / 2)));

	int windowStartPositionY = static_cast<int>((GetSystemMetrics(SM_CYSCREEN) *
		(((GetSystemMetrics(SM_CYSCREEN) - height)
			/ (float)GetSystemMetrics(SM_CYSCREEN)) / 2)));
																	//[https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa]
	m_WindowHandle = CreateWindowEx(0,								//Extended window styles (bits).
					 m_ClassName.c_str(),							//The window class name.
					 windowTitle.c_str(),							//The window title.
					 WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,		//The window styles defining the window appearance.
					 windowStartPositionX,							//Start x-position for window.
					 windowStartPositionY,							//Start y-position for window.
					 windowRect.right - windowRect.left,			//Width of window.
					 windowRect.bottom - windowRect.top,			//Height of window.
					 nullptr,										//Handle to window parent.								
					 nullptr,										//Handle to menu.
					 GetModuleHandle(nullptr),						//Handle to the instance that contains the window procedure for class. 
					 nullptr);										//Pointer to value passed to the window through CREATESTRUCT structure.

	m_Title = windowTitle;
	m_Width = width;
	m_Height = height;

	ShowWindow(m_WindowHandle, SW_SHOWNORMAL);

	//DirectX initialization:
	Graphics::Initialize(m_Width, m_Height);
}

const uint32_t Window::GetWidth() noexcept
{
	return m_Width;
}

const uint32_t Window::GetHeight() noexcept
{
	return m_Height;
}

std::pair<uint32_t, uint32_t> Window::GetWindowResolution() noexcept
{
	return std::pair<uint32_t, uint32_t>(m_Width, m_Height);
}

const bool Window::OnUpdate()
{
	while (PeekMessage(&m_WindowMessage, nullptr, 0u, 0u, PM_REMOVE))
	{
		TranslateMessage(&m_WindowMessage);
		DispatchMessage(&m_WindowMessage);
	}

	HR_I(Graphics::GetSwapChain()->Present(0u, 0u));
	return m_Running;
}

LRESULT Window::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_CLOSE:
	{
		m_Running = false;
		break;
	}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Window::ShutDown()
{
	//BOOL result = UnregisterClass(m_ClassName.c_str(), GetModuleHandle(nullptr));
	//RLS_ASSERT(result != FALSE, "Failed to unregister class.");
}
