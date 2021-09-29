#pragma once
class Shape
{
public:
	Shape() noexcept = default;
	virtual ~Shape() noexcept = default;
};

class Cube : public Shape
{
public:
	Cube() noexcept = default;
	virtual ~Cube() noexcept override = default;
private:
	std::byte m_Bytes[100];
};

class Sphere : public Shape
{
public:
	Sphere() noexcept = default;
	virtual ~Sphere() noexcept override = default;
private:
	std::byte m_Bytes[10000];
};

class Pyramid : public Shape
{
public:
	Pyramid() noexcept = default;
	virtual ~Pyramid() noexcept override = default;
private:
	std::byte m_Bytes[1457];
};