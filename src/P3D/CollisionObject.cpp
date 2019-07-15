#include <Core/MemoryStream.h>
#include <P3D/CollisionObject.h>
#include <glm/gtc/type_ptr.inl>
#include <iostream>

namespace Donut::P3D
{
std::unique_ptr<CollisionObject> CollisionObject::Load(const P3DChunk& chunk)
{
	assert(chunk.IsType(ChunkType::CollisionObject));

	MemoryStream stream(chunk.GetData());

	std::string name     = stream.ReadLPString();
	uint32_t version     = stream.Read<uint32_t>();
	std::string matName  = stream.ReadLPString();
	uint32_t objectCount = stream.Read<uint32_t>();
	uint32_t ownerCount  = stream.Read<uint32_t>();

	for (auto const& volumeChunk : chunk.GetChildren())
	{
		if (!volumeChunk->IsType(ChunkType::CollisionVolume))
			continue;

		auto volume = CollisionVolume::Load(*volumeChunk);
	}

	return std::make_unique<CollisionObject>(name, nullptr);
}

CollisionBoundingBox CollisionBoundingBox::Load(const P3DChunk& chunk)
{
	assert(chunk.IsType(ChunkType::CollisionOrientedBoundingBox));

	auto const size = glm::make_vec3(reinterpret_cast<const float*>(chunk.GetData().data()));

	// should be: Centre, RotX, RotY, RotZ
	auto& children = chunk.GetChildren();
	assert(children.size() == 4);

	auto const centre = glm::make_vec3(reinterpret_cast<const float*>(children.at(0)->GetData().data()));
	auto const rotX   = glm::make_vec3(reinterpret_cast<const float*>(children.at(0)->GetData().data()));
	auto const rotY   = glm::make_vec3(reinterpret_cast<const float*>(children.at(0)->GetData().data()));
	auto const rotZ   = glm::make_vec3(reinterpret_cast<const float*>(children.at(0)->GetData().data()));

	return CollisionBoundingBox(size, centre, rotX, rotY, rotZ);
}

std::unique_ptr<CollisionVolume> CollisionVolume::Load(const P3DChunk& chunk)
{
	assert(chunk.IsType(ChunkType::CollisionVolume));

	// std::cout << chunk.GetChildren().size() << "\n";

	return std::unique_ptr<CollisionVolume>();
}

} // namespace Donut::P3D
