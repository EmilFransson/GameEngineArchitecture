#include "pch.h"
#include "RenderCommand.h"
#include "Graphics.h"
#include "DXDebug.h"
void RenderCommand::ClearBackBuffer(const FLOAT clearColor[4]) noexcept
{
	CHECK_STD(Graphics::GetContext()->ClearRenderTargetView(Graphics::GetBackBufferRTV().Get(), clearColor));
}

void RenderCommand::ClearDepthBuffer() noexcept
{
	CHECK_STD(Graphics::GetContext()->ClearDepthStencilView(Graphics::GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u));
}

void RenderCommand::BindBackBuffer() noexcept
{
	CHECK_STD(Graphics::GetContext()->OMSetRenderTargets(1u, Graphics::GetBackBufferRTV().GetAddressOf(), nullptr));
}

void RenderCommand::UnbindRenderTargets(const unsigned int nrOfTargets) noexcept
{
	static const Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullRTV[] = { nullptr };
	CHECK_STD(Graphics::GetContext()->OMSetRenderTargets(nrOfTargets, nullRTV->GetAddressOf(), nullptr));
}

void RenderCommand::DrawIndexed(const unsigned int indexCount) noexcept
{
	CHECK_STD(Graphics::GetContext()->DrawIndexed(indexCount, 0u, 0u));
}

void RenderCommand::Drawinstanced(const uint32_t vertexCount, const uint32_t instanceCount) noexcept
{
	CHECK_STD(Graphics::GetContext()->DrawInstanced(vertexCount, instanceCount, 0u, 0u));
}

void RenderCommand::SetTopolopy(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept
{
	CHECK_STD(Graphics::GetContext()->IASetPrimitiveTopology(topology));
}