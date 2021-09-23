#include "pch.h"
#include "System.h"
#include "DXDebug.h"
#include "Utility.h"
const System System::m_sInstance;
std::vector<AdapterData> System::m_sAdapters{};
AdapterData System::m_sPrimaryAdapter{};
SystemMetrics System::m_sSystemMetrics{};

const System& System::Get() noexcept
{
	return m_sInstance;
}

void System::Initialize() noexcept
{
	InitializeAdapterMetrics();
	InitializeAdapterOutputMetrics();

	/*Verifying the integrity of the adapter/monitor combination.*/

	assert(m_sPrimaryAdapter.m_DisplayModeList[m_sPrimaryAdapter.m_NrOfDisplaymodes - 1].Width == static_cast<UINT>(GetSystemMetrics(SM_CXSCREEN))
		&& m_sPrimaryAdapter.m_DisplayModeList[m_sPrimaryAdapter.m_NrOfDisplaymodes - 1].Height == static_cast<UINT>(GetSystemMetrics(SM_CYSCREEN)));

	m_sSystemMetrics.m_PrimaryMonitorWidth = GetSystemMetrics(SM_CXSCREEN);
	m_sSystemMetrics.m_PrimaryMonitorHeight = GetSystemMetrics(SM_CYSCREEN);
	m_sSystemMetrics.m_RefreshRateNumerator = m_sPrimaryAdapter.m_DisplayModeList[m_sPrimaryAdapter.m_NrOfDisplaymodes - 1].RefreshRate.Numerator;
	m_sSystemMetrics.m_RefreshRateDenominator = m_sPrimaryAdapter.m_DisplayModeList[m_sPrimaryAdapter.m_NrOfDisplaymodes - 1].RefreshRate.Denominator;
}

const SystemMetrics& System::GetMetrics() noexcept
{
	return m_sSystemMetrics;
}

const Microsoft::WRL::ComPtr<IDXGIAdapter>& System::GetPrimaryAdapter() noexcept
{
	return m_sPrimaryAdapter.m_pAdapter;
}

void System::InitializeAdapterMetrics() noexcept
{
	Microsoft::WRL::ComPtr<IDXGIFactory> pFactory = nullptr;
	HR(CreateDXGIFactory(__uuidof(IDXGIFactory), &pFactory));

	Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter = nullptr;
	UINT index{ 0u };
	while (SUCCEEDED(pFactory->EnumAdapters(index, &pAdapter)))
	{
		m_sAdapters.push_back(AdapterData(pAdapter));
		index++;
	}
	assert(m_sAdapters.size() > 0);
}

void System::InitializeAdapterOutputMetrics() noexcept
{
	assert(m_sAdapters.size() > 0);
	for (auto& adapter : m_sAdapters)
	{
		/*Only dedicated graphics adapters should enumerate outputs.*/
		if (adapter.m_AdapterDescriptor.DedicatedVideoMemory > 0)
		{
			HR(adapter.m_pAdapter->EnumOutputs(0u, &adapter.m_pAdapterOutput));
			//[https://docs.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgioutput-getdisplaymodelist]
			HR(adapter.m_pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,		//Color format of monitor.
															DXGI_ENUM_MODES_INTERLACED,		//Flags for enumerating display modes.
															&adapter.m_NrOfDisplaymodes,	//Number of display modes available for this monitor/adapter combination. 
															nullptr));						//The display mode structure(s) to be filled.

/*Create a list to hold all the possible display modes for this monitor/video card combination.*/
			adapter.m_DisplayModeList = std::shared_ptr<DXGI_MODE_DESC[]>(DBG_NEW DXGI_MODE_DESC[adapter.m_NrOfDisplaymodes]);
			// Now fill the display mode list structures.
			HR(adapter.m_pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, //See above for reference.
				DXGI_ENUM_MODES_INTERLACED,
				&adapter.m_NrOfDisplaymodes,
				adapter.m_DisplayModeList.get()));
		}
		if (adapter.m_AdapterDescriptor.DedicatedVideoMemory > m_sPrimaryAdapter.m_AdapterDescriptor.DedicatedVideoMemory)
		{
			m_sPrimaryAdapter = adapter;
		}
	}
	assert(m_sPrimaryAdapter.m_pAdapter != nullptr);
}
