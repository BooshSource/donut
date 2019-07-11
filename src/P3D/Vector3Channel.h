#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>

namespace Donut::P3D {

class Vector3Channel {
public:
	Vector3Channel() {}
private:
	uint32_t _version;
	uint32_t _numberOfFrames;
	std::string _parameter;
	std::vector<glm::vec3> _values;
	std::vector<uint16_t> _frames;
};

} // namespace Donut::P3D
