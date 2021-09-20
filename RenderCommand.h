#pragma once
class RenderCommand
{
public:
	static void ClearBackBuffer(const FLOAT clearColor[4]) noexcept;
	static void ClearDepthBuffer() noexcept;
	static void BindBackBuffer() noexcept;
	static void UnbindRenderTargets(const unsigned int nrOfTargets = 1u) noexcept;
	static void DrawIndexed(const unsigned int indexCount) noexcept;
	static void Drawinstanced(const uint32_t vertexCount, const uint32_t instanceCount) noexcept;
	static void SetTopolopy(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept;
};