#include "pch.h"
#include "Graphics.h"
#include "DXDebug.h"
#include "System.h"
#include "Utility.h"
const Graphics Graphics::m_sInstance;
Microsoft::WRL::ComPtr<ID3D11Device> Graphics::m_pDevice = nullptr;
Microsoft::WRL::ComPtr<ID3D11DeviceContext> Graphics::m_pContext = nullptr;
Microsoft::WRL::ComPtr<IDXGISwapChain> Graphics::m_pSwapChain = nullptr;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Graphics::m_pBackBufferRTV = nullptr;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> Graphics::m_pDepthStencilView = nullptr;

const Graphics& Graphics::Get() noexcept
{
	return m_sInstance;
}

void Graphics::Initialize(const unsigned int windowClientWidth, const unsigned int windowClientHeight) noexcept
{
	DXDebug::Get().InitializeDXGIInfoQueue();
	UINT flags{};
#if defined(DEBUG) | defined (_DEBUG)
	flags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif
	DXGI_SWAP_CHAIN_DESC swapChainDescriptor = { 0 };
	swapChainDescriptor.BufferDesc.Width = windowClientWidth;
	swapChainDescriptor.BufferDesc.Height = windowClientHeight;
	swapChainDescriptor.BufferDesc.RefreshRate.Numerator = System::GetMetrics().m_RefreshRateNumerator;
	swapChainDescriptor.BufferDesc.RefreshRate.Denominator = System::GetMetrics().m_RefreshRateDenominator;
	swapChainDescriptor.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDescriptor.SampleDesc.Count = 1u;
	swapChainDescriptor.SampleDesc.Quality = 0u;
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescriptor.BufferCount = 2u;
	swapChainDescriptor.OutputWindow = GetActiveWindow();
	swapChainDescriptor.Windowed = TRUE;
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescriptor.Flags = 0u;

	const D3D_FEATURE_LEVEL requiredFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL supportedFeatureLevel = {};

	HR_I(D3D11CreateDeviceAndSwapChain(System::GetPrimaryAdapter().Get(),
									   D3D_DRIVER_TYPE_UNKNOWN,
									   nullptr,
									   flags,
									   &requiredFeatureLevel,
									   1u,
									   D3D11_SDK_VERSION,
									   &swapChainDescriptor,
									   &m_pSwapChain,
									   &m_pDevice,
									   &supportedFeatureLevel,
									   &m_pContext));
	assert(supportedFeatureLevel == requiredFeatureLevel);

	DXDebug::InitializeDefaultInfoQueue(m_pDevice);

	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer{ nullptr };
	HR_I(m_pSwapChain->GetBuffer(0u, __uuidof(ID3D11Resource), &pBackBuffer));
	HR_I(m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_pBackBufferRTV));

	D3D11_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
	depthStencilDescriptor.DepthEnable = TRUE;
	depthStencilDescriptor.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDescriptor.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
	depthStencilDescriptor.StencilEnable = FALSE;
	depthStencilDescriptor.StencilReadMask = 0u;
	depthStencilDescriptor.StencilWriteMask = 0u;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthStencilState{ nullptr };
	HR_I(Graphics::GetDevice()->CreateDepthStencilState(&depthStencilDescriptor, &pDepthStencilState));
	CHECK_STD(Graphics::GetContext()->OMSetDepthStencilState(pDepthStencilState.Get(), 1u));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencilTexture{ nullptr };
	D3D11_TEXTURE2D_DESC depthStencilTextureDescriptor = {};
	depthStencilTextureDescriptor.Width = windowClientWidth;
	depthStencilTextureDescriptor.Height = windowClientHeight;
	depthStencilTextureDescriptor.MipLevels = 1u;
	depthStencilTextureDescriptor.ArraySize = 1u;
	depthStencilTextureDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	depthStencilTextureDescriptor.SampleDesc.Count = 1u;
	depthStencilTextureDescriptor.SampleDesc.Quality = 0u;
	depthStencilTextureDescriptor.Usage = D3D11_USAGE_DEFAULT; //Immutable??
	depthStencilTextureDescriptor.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
	depthStencilTextureDescriptor.CPUAccessFlags = 0u;
	depthStencilTextureDescriptor.MiscFlags = 0u;
	HR_I(Graphics::GetDevice()->CreateTexture2D(&depthStencilTextureDescriptor, nullptr, &pDepthStencilTexture));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescriptor = {};
	depthStencilViewDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDescriptor.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDescriptor.Texture2D.MipSlice = 0u;
	HR_I(Graphics::GetDevice()->CreateDepthStencilView(pDepthStencilTexture.Get(), &depthStencilViewDescriptor, &m_pDepthStencilView));
}

const Microsoft::WRL::ComPtr<ID3D11Device>& Graphics::GetDevice() noexcept
{
	return m_pDevice;
}

const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& Graphics::GetContext() noexcept
{
	return m_pContext;
}

const Microsoft::WRL::ComPtr<IDXGISwapChain>& Graphics::GetSwapChain() noexcept
{
	return m_pSwapChain;
}

const Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& Graphics::GetBackBufferRTV() noexcept
{
	return m_pBackBufferRTV;
}

const Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& Graphics::GetDepthStencilView() noexcept
{
	return m_pDepthStencilView;
}