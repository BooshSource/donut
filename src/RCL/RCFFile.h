#pragma once

#include <P3D/P3DChunk.h>
#include <Core/MemoryStream.h>
#include <string>
#include <map>

namespace Donut::RCL
{
	enum class FileTypes : uint32_t
	{
		RSD = 0x3F1341B4,
	};

	struct FileEntry
	{
		uint32_t hash;
		uint32_t offset;
		uint32_t size;
	};

	class RCFFile
	{
	public:

		RCFFile(const std::string& file);

		const std::string& GetFileName() const
		{
			return _filename;
		}

		std::unique_ptr<MemoryStream> GetFileStream(const std::string name);
		std::unique_ptr<MemoryStream> GetFileStream(uint32_t hash);

		const std::vector<std::string>& GetFilenames() const { return _filenames; }

	protected:

		std::string _filename;
		std::map<uint32_t, FileEntry> _fileEntries;
		std::vector<std::string> _filenames;
		std::map<std::string, uint32_t> _filenameHashes;
	};
} // namespace Donut::RCL
