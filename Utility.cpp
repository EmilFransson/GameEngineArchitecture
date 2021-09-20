#include "pch.h"
#include "Utility.h"
#pragma warning(disable : 4996)
std::string ConvertWstringToString(std::wstring const& wstr) noexcept
{
	std::size_t size = sizeof(wstr);
	char* str = DBG_NEW char[size];
	std::string temp;

	std::wcstombs(str, wstr.c_str(), size);

	temp = str;
	delete[] str;

	return temp;
}
std::wstring ConvertStringToWstring(const std::string& string) noexcept
{
	int size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, string.c_str(), static_cast<int>(string.size()), nullptr, 0);
	assert(size > 0);
	std::wstring converted{};
	converted.resize(size);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, string.c_str(), static_cast<int>(string.size()), &(*converted.begin()), static_cast<int>(converted.size()));

	return converted;
}