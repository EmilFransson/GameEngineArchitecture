#pragma once
struct AdapterData
{
	AdapterData(Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter = nullptr)
		: m_pAdapter{ pAdapter }
	{
		if (pAdapter != nullptr)
		{
			HRESULT hr = m_pAdapter->GetDesc(&m_AdapterDescriptor);
			assert(!FAILED(hr));
		}
	}
	Microsoft::WRL::ComPtr<IDXGIAdapter> m_pAdapter{ nullptr };
	Microsoft::WRL::ComPtr<IDXGIOutput> m_pAdapterOutput{ nullptr };
	DXGI_ADAPTER_DESC m_AdapterDescriptor{};
	std::shared_ptr<DXGI_MODE_DESC[]> m_DisplayModeList{ nullptr };
	UINT m_NrOfDisplaymodes{};
};

struct SystemMetrics
{
	unsigned int m_PrimaryMonitorWidth{ 0u };
	unsigned int m_PrimaryMonitorHeight{ 0u };
	unsigned int m_RefreshRateNumerator{ 0u };
	unsigned int m_RefreshRateDenominator{ 0u };
};

class System
{
public:
	[[nodiscard]] static const System& Get() noexcept;
	static void Initialize() noexcept;
	[[nodiscard]] static const SystemMetrics& GetMetrics() noexcept;
	[[nodiscard]] static const Microsoft::WRL::ComPtr<IDXGIAdapter>& GetPrimaryAdapter() noexcept;
private:
	static void InitializeAdapterMetrics() noexcept;
	static void InitializeAdapterOutputMetrics() noexcept;
private:
	static const System m_sInstance;
	static std::vector<AdapterData> m_sAdapters;
	static AdapterData m_sPrimaryAdapter;
	static SystemMetrics m_sSystemMetrics;
};