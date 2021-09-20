#pragma once

#pragma comment(lib, "dxguid.lib")

class DXDebug
{
public:
	[[nodiscard]] static const DXDebug& Get() noexcept;
	static void InitializeDXGIInfoQueue() noexcept;
	static void InitializeDefaultInfoQueue(Microsoft::WRL::ComPtr<ID3D11Device> pDevice) noexcept;
	static void ClearDXGIInfoQueue() noexcept;
	static void ClearDefaultInfoQueue() noexcept;
	static void PrintDXGIMessages() noexcept;
	static void PrintInfoQueueMessages() noexcept;
	[[nodiscard]] static const bool HasDXGIInfoQueueMessages() noexcept;
	[[nodiscard]] static Microsoft::WRL::ComPtr<ID3D11InfoQueue>& GetInfoQueue() noexcept;
	[[nodiscard]] static Microsoft::WRL::ComPtr<IDXGIInfoQueue>& GetDXGIInfoQueue() noexcept;
private:
	static const DXDebug m_sInstance;
	static Microsoft::WRL::ComPtr<ID3D11InfoQueue> m_pInfoQueue;
	static Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_pDXGIInfoQueue;
};

#if defined (DEBUG) | defined (_DEBUG)
#ifndef HR
#define HR(function)	\
		{	\
			HRESULT hr = (function);	\
			if (FAILED(hr))	\
			{	\
				std::cout << "Error: DirectX 11 has encountered a critical error:\n"; \
				std::cout << "Debug information:\nFile:" << __FILE__ << "\nFunction: " <<  __FUNCTION__ << "\nLine: " << __LINE__ << "\n"; \
				std::cout << std::string(ConvertWstringToString(_com_error(hr).ErrorMessage())) << "\n"; \
				std::cout << "HRESULT not successful.\n"; \
				assert(false); \
			} \
		}
#endif
#else
#ifndef HR
#define HR(function) function
#endif
#endif

#if defined (DEBUG) | defined (_DEBUG)
#ifndef HR_I
#define HR_I(function)	\
			{	\
				assert(DXDebug::GetDXGIInfoQueue() != nullptr);	\
				HRESULT hr = (function);	\
				if (FAILED(hr))	\
				{	\
					std::cout << "Error: DirectX 11 has encountered a critical error:\n"; \
					std::cout << "Debug information:\nFile:" << __FILE__ << "\nFunction: " <<  __FUNCTION__ << "\nLine: " << __LINE__ << "\n"; \
					std::cout << std::string(ConvertWstringToString(_com_error(hr).ErrorMessage())) << "\n"; \
					DXDebug::PrintDXGIMessages();	\
					std::cout << "HRESULT not successful.\n"; \
					assert(false); \
				} \
			}
#endif
#else
#ifndef HR_I
#define HR_I(function) function
#endif
#endif

#if defined (DEBUG) | defined (_DEBUG)
#ifndef CHECK_STD
#define CHECK_STD(function)	\
			{	\
				assert(DXDebug::GetDXGIInfoQueue() != nullptr);	\
				DXDebug::ClearDXGIInfoQueue();	\
				(function);	\
				if (DXDebug::HasDXGIInfoQueueMessages())	\
				{	\
					std::cout << "Error: DirectX 11 has encountered a critical error:\n"; \
					std::cout << "Debug information:\nFile:" << __FILE__ << "\nFunction: " <<  __FUNCTION__ << "\nLine: " << __LINE__ << "\n"; \
					DXDebug::PrintDXGIMessages();	\
					std::cout << "Function call resulted in undefined behaviour."; \
					assert(false); \
				} \
			}
#endif
#else
#ifndef CHECK_STD
#define CHECK_STD(function) function
#endif
#endif

#if defined (DEBUG) | defined (_DEBUG)
#ifndef COMPILE_PIXELSHADER_FROM_FILE
#define COMPILE_PIXELSHADER_FROM_FILE(fileName, shaderBlob) \
		{	\
			assert(DXDebug::GetDXGIInfoQueue() != nullptr);	\
			DXDebug::ClearDXGIInfoQueue();	\
			Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob = nullptr;	\
			HRESULT hr = D3DCompileFromFile(fileName, \
											nullptr, \
											D3D_COMPILE_STANDARD_FILE_INCLUDE, \
											"ps_main", \
											"ps_5_0", \
											D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, \
											0u, \
											&shaderBlob, \
											&pErrorBlob);	\
				if (FAILED(hr))	\
				{	\
					std::cout << "Error: DirectX 11 has encountered a critical error:\n"; \
					std::cout << "Debug information:\nFile:" << __FILE__ << "\nFunction: " <<  __FUNCTION__ << "\nLine: " << __LINE__ << "\n"; \
					std::cout << std::string(ConvertWstringToString(_com_error(hr).ErrorMessage())) << "\n"; \
					DXDebug::PrintDXGIMessages();	\
					if (pErrorBlob != nullptr)	\
					{	\
						std::cout << (char*)pErrorBlob->GetBufferPointer();	\
					}	\
						std::cout << "HRESULT not successful\n"; \
						assert(false); \
				}	\
		}
#endif
#else
#ifndef COMPILE_PIXELSHADER_FROM_FILE
#define COMPILE_PIXELSHADER_FROM_FILE(fileName, shaderBlob) \
		{	\
			D3DCompileFromFile(fileName, \
							   nullptr, \
							   D3D_COMPILE_STANDARD_FILE_INCLUDE, \
							   "ps_main", \
							   "ps_5_0", \
							   D3DCOMPILE_ENABLE_STRICTNESS, \
							   0u, \
							   &shaderBlob, \
							   nullptr);	\
		}
#endif
#endif

#if defined (DEBUG) | defined (_DEBUG)
#ifndef COMPILE_VERTEXSHADER_FROM_FILE
#define COMPILE_VERTEXSHADER_FROM_FILE(fileName, shaderBlob) \
		{	\
			assert(DXDebug::GetDXGIInfoQueue() != nullptr);	\
			DXDebug::ClearDXGIInfoQueue();	\
			Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob = nullptr;	\
			HRESULT hr = D3DCompileFromFile(fileName, \
											nullptr, \
											D3D_COMPILE_STANDARD_FILE_INCLUDE, \
											"vs_main", \
											"vs_5_0", \
											D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, \
											0u, \
											&shaderBlob, \
											&pErrorBlob);	\
				if (FAILED(hr))	\
				{	\
					std::cout << "Error: DirectX 11 has encountered a critical error:\n"; \
					std::cout << "Debug information:\nFile:" << __FILE__ << "\nFunction: " <<  __FUNCTION__ << "\nLine: " << __LINE__ << "\n"; \
					std::cout << std::string(ConvertWstringToString(_com_error(hr).ErrorMessage())) << "\n"; \
					DXDebug::PrintDXGIMessages();	\
					if (pErrorBlob != nullptr)	\
					{	\
						std::cout << (char*)pErrorBlob->GetBufferPointer();	\
					}	\
						std::cout << "HRESULT not successful\n"; \
						assert(false);\
				}	\
		}
#endif
#else
#ifndef COMPILE_VERTEXSHADER_FROM_FILE
#define COMPILE_VERTEXSHADER_FROM_FILE(fileName, shaderBlob) \
		{	\
			D3DCompileFromFile(fileName, \
							   nullptr, \
							   D3D_COMPILE_STANDARD_FILE_INCLUDE, \
							   "vs_main", \
							   "vs_5_0", \
							   D3DCOMPILE_ENABLE_STRICTNESS, \
							   0u, \
							   &shaderBlob, \
							   nullptr);	\
		}
#endif
#endif