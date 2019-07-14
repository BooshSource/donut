#pragma once

#include "AnimChannel.h"

#include <Core/MemoryStream.h>
#include <glm/mat4x4.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Donut::P3D
{

class Vector3Channel: AnimChannel
{
  public:
	Vector3Channel() {}
	virtual void Read(MemoryStream& stream) override;

	uint32_t GetNumFrames() const { return _numberOfFrames; }
	const std::vector<glm::vec3>& GetValues() const { return _values; }
	const std::vector<uint16_t>& GetFrames() const { return _frames; }

  private:
	uint32_t _numberOfFrames;
	std::vector<glm::vec3> _values;
	std::vector<uint16_t> _frames;
};

} // namespace Donut::P3D
