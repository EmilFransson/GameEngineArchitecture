#include "pch.h"
#include "DXDebug.h"
#include "Utility.h"

const DXDebug DXDebug::m_sInstance;
Microsoft::WRL::ComPtr<ID3D11InfoQueue> DXDebug::m_pInfoQueue{ nullptr };
Microsoft::WRL::ComPtr<IDXGIInfoQueue> DXDebug::m_pDXGIInfoQueue{ nullptr };

const DXDebug& DXDebug::Get() noexcept
{
	return m_sInstance;
}

void DXDebug::InitializeDXGIInfoQueue() noexcept
{
	typedef HRESULT(WINAPI* LPDXGIGETDEBUGINTERFACE)(REFIID, void**);
	HMODULE dxgidebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (dxgidebug)
	{
		auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(reinterpret_cast<void*>(GetProcAddress(dxgidebug, "DXGIGetDebugInterface")));
		if (!SUCCEEDED(dxgiGetDebugInterface(IID_PPV_ARGS(&m_pDXGIInfoQueue))))
		{
			std::cout << "Failed to get DXGI debug interface.\n";
			assert(false);
		}
	}
	else
	{
		std::cout << "Failed to load DXGI-Debug dll.\n";
		assert(false);
	}
}

void DXDebug::InitializeDefaultInfoQueue(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept
{
	HR(pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), &m_pInfoQueue));
}

void DXDebug::ClearDXGIInfoQueue() noexcept
{
	m_pDXGIInfoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
}

void DXDebug::ClearDefaultInfoQueue() noexcept
{
	m_pInfoQueue->ClearStoredMessages();
}

void DXDebug::PrintDXGIMessages() noexcept
{
	for (UINT64 i{ 0u }; i < m_pDXGIInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL); ++i)
	{
		SIZE_T messageSize{};
		HR(m_pDXGIInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageSize));
		std::unique_ptr<DXGI_INFO_QUEUE_MESSAGE> message = std::unique_ptr<DXGI_INFO_QUEUE_MESSAGE>(DBG_NEW DXGI_INFO_QUEUE_MESSAGE[messageSize]);
		HR(m_pDXGIInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, message.get(), &messageSize));
		std::cout << message->pDescription << "\n";
	}
}

void DXDebug::PrintInfoQueueMessages() noexcept
{
	for (UINT64 i{ 0u }; i < m_pInfoQueue->GetNumStoredMessages(); ++i)
	{
		SIZE_T messageSize{};
		HR(m_pInfoQueue->GetMessage(i, nullptr, &messageSize));
		std::unique_ptr<D3D11_MESSAGE> message = std::unique_ptr<D3D11_MESSAGE>(DBG_NEW D3D11_MESSAGE[messageSize]);
		HR(m_pInfoQueue->GetMessage(i, message.get(), &messageSize));
		std::cout << message->pDescription << "\n";
	}
}

const bool DXDebug::HasDXGIInfoQueueMessages() noexcept
{
	if (m_pDXGIInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL) > 0)
	{
		return true;
	}
	return false;
}

Microsoft::WRL::ComPtr<ID3D11InfoQueue>& DXDebug::GetInfoQueue() noexcept
{
	return m_pInfoQueue;
}

Microsoft::WRL::ComPtr<IDXGIInfoQueue>& DXDebug::GetDXGIInfoQueue() noexcept
{
	return m_pDXGIInfoQueue;
}