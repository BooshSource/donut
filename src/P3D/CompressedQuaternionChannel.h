#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glm/mat4x4.hpp>
#include "AnimChannel.h"

namespace Donut::P3D {

class CompressedQuaternionChannel : AnimChannel {
public:
	CompressedQuaternionChannel() {}
	virtual void Read(MemoryStream& stream) override;

	uint32_t GetNumFrames() { return _numberOfFrames; }
private:
	uint32_t _numberOfFrames;
	std::vector<glm::vec4> _values;
	std::vector<uint16_t> _frames;
};

} // namespace Donut::P3D
