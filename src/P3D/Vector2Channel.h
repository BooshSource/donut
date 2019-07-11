#pragma once

#include "AnimChannel.h"

#include <glm/mat4x4.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Donut::P3D
{

class Vector2Channel: AnimChannel
{
  public:
	Vector2Channel() {}
	virtual void Read(MemoryStream& stream) override;

	uint32_t GetNumFrames() { return _numberOfFrames; }
	std::vector<glm::vec2>& GetValues() { return _values; }
	std::vector<uint16_t>& GetFrames() { return _frames; }

  private:
	uint32_t _numberOfFrames;
	uint16_t _mapping;
	std::vector<glm::vec2> _values;
	std::vector<uint16_t> _frames;
	glm::vec3 _constants;
};

} // namespace Donut::P3D
