#pragma once

#include <filesystem>
#include <string>
#include <cstdio>
#include <cassert>

namespace Donut
{

enum class FileMode
{
	Read,
	Write
};

enum class FileSeekMode
{
	Begin = SEEK_SET,
	Current = SEEK_CUR,
	End = SEEK_END
};

class File
{
public:
	File();
	File(const std::filesystem::path& filename, FileMode mode);
	~File();

	void Open(const std::filesystem::path& filename, FileMode mode);
	void Close();

	template <typename T>
	std::size_t Read(T* data) const
	{
		assert(_file != nullptr);
		static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
		return std::fread(data, sizeof(T), 1, _file);
	}

	template <typename T>
	std::size_t Read(T* data, std::size_t length) const
	{
		assert(_file != nullptr);
		static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
		return std::fread(data, sizeof(T), length, _file);
	}

	template <typename T>
	std::size_t ReadBytes(T* data, std::size_t length) const
	{
		assert(_file != nullptr);
		static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
		return Read(reinterpret_cast<uint8_t*>(data), length);
	}

	void Seek(size_t position, FileSeekMode mode) const;
	size_t Position() const;
	size_t Size() const;
	void Flush();
protected:
	FILE* _file;
};

}
