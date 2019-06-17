#include "Framework.h"
#include "FileStream.h"

FileStream::FileStream(const std::string & path, const StreamMode & mode)
{
	streamMode = mode;
	bOpen = false;

	switch (streamMode)
	{
	case StreamMode::Write:
		out.open(path, std::ios::out | std::ios::binary);
		if (out.fail())
		{
			Log::ErrorF("FileStream::FileStream : Failed to open \"%s\" for writing", path.c_str());
			return;
		}
		break;
	case StreamMode::Read:
		in.open(path, std::ios::in | std::ios::binary);
		if (in.fail())
		{
			Log::ErrorF("FileStream::FileStream : Failed to open \"%s\" for reading", path.c_str());
			return;
		}
		break;
	}

	bOpen = true;
}

FileStream::~FileStream()
{
	switch (streamMode)
	{
	case StreamMode::Write:
		out.flush();
		out.close();
		break;
	case StreamMode::Read:
		in.clear();
		in.close();
		break;
	}
}

void FileStream::Write(const std::string & value)
{
	auto length = static_cast<uint>(value.length());
	Write(length);

	out.write(value.c_str(), length);
}

void FileStream::Write(const std::vector<VertexModel>& value)
{
	auto length = static_cast<uint>(value.size());
	Write(length);

	out.write(reinterpret_cast<const char*>(value.data()), sizeof(VertexModel) * length);
}

void FileStream::Write(const std::vector<uint>& value)
{
	auto length = static_cast<uint>(value.size());
	Write(length);

	out.write(reinterpret_cast<const char*>(value.data()), sizeof(uint) * length);
}

void FileStream::Write(const std::vector<std::byte>& value)
{
	auto length = static_cast<uint>(value.size());
	Write(length);

	out.write(reinterpret_cast<const char*>(value.data()), sizeof(std::byte) * length);
}

void FileStream::Write(const void * value, const uint & size)
{
	out.write(reinterpret_cast<const char*>(value), size);
}

void FileStream::Read(std::string & value)
{
	auto length = Read<uint>();

	value.clear();
	value.shrink_to_fit();
	value.resize(length);
	in.read(const_cast<char*>(value.c_str()), length);
}

void FileStream::Read(std::vector<VertexModel>& value)
{
	auto length = Read<uint>();

	value.clear();
	value.shrink_to_fit();
	value.reserve(length);
	value.resize(length);

	in.read(reinterpret_cast<char*>(value.data()), sizeof(VertexModel) * length);
}

void FileStream::Read(std::vector<uint>& value)
{
	auto length = Read<uint>();

	value.clear();
	value.shrink_to_fit();
	value.reserve(length);
	value.resize(length);

	in.read(reinterpret_cast<char*>(value.data()), sizeof(uint) * length);
}

void FileStream::Read(std::vector<std::byte>& value)
{
	auto length = Read<uint>();

	value.clear();
	value.shrink_to_fit();
	value.reserve(length);
	value.resize(length);

	in.read(reinterpret_cast<char*>(value.data()), sizeof(std::byte) * length);
}