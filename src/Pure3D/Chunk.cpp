#include <Pure3D/Chunk.h>

#include <iostream>

namespace Donut::Pure3D
{

Chunk::Chunk()
{

}

Chunk::~Chunk()
{

}

// indentLevel is for debug, remove it after
void Chunk::Read(File& file, int indentLevel)
{
	file.Read<uint32_t>(&_type, 1);
	file.Read<uint32_t>(&_dataSize, 1);
	file.Read<uint32_t>(&_totalSize, 1);

	std::cout << std::hex << std::showbase <<
		std::string(indentLevel, '\t') + "Type: " << _type << "\n"
		+ std::string(indentLevel, '\t') + "Data Size: " << _dataSize << "\n"
		+ std::string(indentLevel, '\t') + "ChildrenSize: " << _totalSize - _dataSize << "\n";

	// seek over the data
	file.Seek(_dataSize - 12, FileSeekMode::Current);

	readChildren(file, indentLevel + 1);
}

void Chunk::readChildren(File& file, int indentLevel)
{
	uint32_t childrenSize = _totalSize - _dataSize;

	if (childrenSize <= 0)
		return;

	uint32_t endPos = file.Position() + childrenSize;
	while (file.Position() < endPos)
	{
		_children.push_back(std::make_unique<Chunk>());
		_children.back()->Read(file, indentLevel);
	}
}

}
