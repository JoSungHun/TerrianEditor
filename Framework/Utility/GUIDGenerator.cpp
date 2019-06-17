#include "Framework.h"
#include "GUIDGenerator.h"
#include <sstream>
#include <iomanip>
#include <guiddef.h>
#include <winerror.h>

const uint GUIDGenerator::Generate()
{
	std::hash<std::string> hasher;

	return static_cast<uint>(hasher(GenerateToString()));
}

std::string GUIDGenerator::GenerateToString()
{
	GUID guid;
	auto hr = CoCreateGuid(&guid);

	if (SUCCEEDED(hr))
	{
		std::stringstream sstream;
		sstream
			<< std::hex << std::uppercase
			<< std::setw(8) << std::setfill('0') << guid.Data1 << "-"
			<< std::setw(4) << std::setfill('0') << guid.Data2 << "-"
			<< std::setw(4) << std::setfill('0') << guid.Data3 << "-"
			<< std::setw(2) << std::setfill('0') << static_cast<int>(guid.Data4[0])
			<< std::setw(2) << std::setfill('0') << static_cast<int>(guid.Data4[1])
			<< std::setw(2) << std::setfill('0') << static_cast<int>(guid.Data4[2])
			<< std::setw(2) << std::setfill('0') << static_cast<int>(guid.Data4[3])
			<< std::setw(2) << std::setfill('0') << static_cast<int>(guid.Data4[4])
			<< std::setw(2) << std::setfill('0') << static_cast<int>(guid.Data4[5])
			<< std::setw(2) << std::setfill('0') << static_cast<int>(guid.Data4[6])
			<< std::setw(2) << std::setfill('0') << static_cast<int>(guid.Data4[7]);

		return sstream.str();
	}

	return "N/A"; //Not Applicable
}
