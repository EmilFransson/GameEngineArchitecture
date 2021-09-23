#pragma once
class Cube
{
public:
	Cube() noexcept = default;
	~Cube() noexcept = default;
private:
	std::byte m_Bytes[100];
};