#include "MaterialParser.h"

#include <fstream>

MaterialParser::MaterialParser(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "MaterialParser: cannot open '" << path << "'\n";
		__debugbreak();
		return;
	}
	const size_t fileSize = static_cast<size_t>(file.tellg());
	file.seekg(0);
	m_Buffer.resize(fileSize);
	file.read(reinterpret_cast<char*>(m_Buffer.data()), fileSize);

	FArchiveRead ar(m_Buffer.data(), m_Buffer.size());
	m_CC.Deserialize(ar);
}

MaterialParser::MaterialParser(const void* data, size_t size)
{
	m_Buffer.assign(static_cast<const uint8_t*>(data),
	                static_cast<const uint8_t*>(data) + size);

	FArchiveRead ar(m_Buffer.data(), m_Buffer.size());
	m_CC.Deserialize(ar);
}
