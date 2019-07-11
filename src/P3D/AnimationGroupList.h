#pragma once

#include <P3D/P3DChunk.h>
#include <P3D/AnimationGroup.h>
#include <memory>

namespace Donut::P3D {

class AnimationGroupList {
public:
	AnimationGroupList(std::vector<std::unique_ptr<AnimationGroup>> groups) :
		_groups(std::move(groups)) {}

	static std::unique_ptr<AnimationGroupList> Load(const P3DChunk&);
private:
	std::vector<std::unique_ptr<AnimationGroup>> _groups;
};

} // namespace Donut::P3D
