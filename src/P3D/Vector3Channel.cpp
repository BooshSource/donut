#include <Core/MemoryStream.h>
#include <P3D/Vector3Channel.h>
#include <iostream>

namespace Donut::P3D
{

void Vector3Channel::Read(MemoryStream& stream)
{
	AnimChannel::Read(stream);

	_numberOfFrames = stream.Read<uint32_t>();

	_frames.resize(_numberOfFrames);
	stream.ReadBytes(reinterpret_cast<uint8_t*>(_frames.data()), _numberOfFrames * sizeof(uint16_t));

	_values.resize(_numberOfFrames);
	stream.ReadBytes(reinterpret_cast<uint8_t*>(_values.data()), _numberOfFrames * sizeof(glm::vec3));
}
} // namespace Donut::P3D
