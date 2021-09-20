#pragma once
class Window
{
public:
	static const Window& Get() noexcept;
	static void Initialize(const std::wstring& windowTitle = L"WindowTitle", const uint32_t width = 1280u, const uint32_t height = 720u);
	[[nodiscard]] static const uint32_t GetWidth() noexcept;
	[[nodiscard]] static const uint32_t GetHeight() noexcept;
	[[nodiscard]] static std::pair<uint32_t, uint32_t> GetWindowResolution() noexcept;
	[[nodiscard]] static const bool OnUpdate();
private:
	Window() noexcept = default;
	~Window() noexcept;
	static LRESULT HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static void ShutDown();
#pragma region Deleted Ctors
	Window(const Window& otherWindow) = delete;
	Window& operator=(const Window& otherWindow) = delete;
	Window(const Window&& otherWindow) = delete;
	Window&& operator=(const Window&& otherWindow) = delete;
#pragma endregion
private:
	static const Window m_sInstance;
	static std::wstring m_Title;
	static std::wstring m_ClassName;
	static uint32_t m_Width;
	static uint32_t m_Height;
	static HWND m_WindowHandle;
	static MSG m_WindowMessage;
	static bool m_Running;
};