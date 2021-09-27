#pragma once
class Cube
{
public:
	Cube() noexcept = default;
	~Cube() noexcept = default;
private:
	std::byte m_Bytes[100];
};

class Sphere
{
public:
	Sphere() noexcept = default;
	~Sphere() noexcept = default;
private:
	std::byte m_Bytes[10000];
};

class Pyramid
{
public:
	Pyramid() noexcept = default;
	~Pyramid() noexcept = default;
private:
	std::byte m_Bytes[1487];
};