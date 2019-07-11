#pragma once

#include <P3D/P3DChunk.h>
#include <string>
#include <memory>

namespace Donut::P3D {

class Animation {
public:
	Animation(std::string name, std::string animType, float numFrames, float frameRate, uint32_t looping) :
		_name(name), _animType(animType), _numFrames(numFrames), _frameRate(frameRate), _looping(looping) {}

	static std::unique_ptr<Animation> Load(const P3DChunk&);

	const std::string& GetName() { return _name; }
	const std::string& GetAnimType() { return _animType; }
	float GetNumFrames() { return _numFrames; }
	float GetFrameRate() { return _frameRate; }
	uint32_t GetLooping() { return _looping; }
private:
	std::string _name;
	std::string _animType;
	float _numFrames;
	float _frameRate;
	uint32_t _looping;
};

} // namespace Donut::P3D
