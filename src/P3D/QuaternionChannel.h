#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace Donut::P3D {

class QuaternionChannel {
public:
	QuaternionChannel() {}
private:
	uint32_t _version;
	uint32_t _numberOfFrames;
	std::string _parameter;
	std::vector<glm::vec4> _values;
	std::vector<uint16_t> _frames;
};

} // namespace Donut::P3D
