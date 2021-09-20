#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class Graphics
{
public:
	[[nodiscard]] static const Graphics& Get() noexcept;
	static void Initialize(const unsigned int windowClientWidth, const unsigned int windowClientHeight) noexcept;
	[[nodiscard]] static const Microsoft::WRL::ComPtr<ID3D11Device>& GetDevice() noexcept;
	[[nodiscard]] static const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetContext() noexcept;
	[[nodiscard]] static const Microsoft::WRL::ComPtr<IDXGISwapChain>& GetSwapChain() noexcept;
	[[nodiscard]] static const Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& GetBackBufferRTV() noexcept;
	[[nodiscard]] static const Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& GetDepthStencilView() noexcept;
private:
	Graphics() noexcept = default;
	~Graphics() noexcept = default;
#pragma region Deleted Ctors
	Graphics(const Graphics&) = delete;
	Graphics(const Graphics&&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	Graphics&& operator=(const Graphics&&) = delete;
#pragma endregion
private:
	static const Graphics m_sInstance;
	static Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
	static Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;
	static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pBackBufferRTV;
	static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
};