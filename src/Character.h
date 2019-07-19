#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <string>
#include <unordered_map>

// #include <Render/OpenGL/TextureBuffer.h>

#include <P3D/Skeleton.h>

namespace Donut
{
namespace GL
{
class ShaderProgram;
class TextureBuffer;
class Texture2D;
} // namespace GL

namespace P3D
{
class Animation;
class PolySkin;
class Shader;
// class Skeleton;
class Texture;
} // namespace P3D

class SkinModel;
class SkinAnimation;
class ResourceManager;

struct SkeletonJoint
{
	std::string name;
	uint32_t parent;
	glm::mat4 restPose;

	SkeletonJoint(std::string name, const uint32_t parent, const glm::mat4 restPose):
	    name(std::move(name)), parent(parent), restPose(restPose)
	{
	}
};

class Character
{
  public:
	Character();
	void LoadModel(const std::string& name);
	void LoadAnimations(const std::string& name);

	// our own methods
	void SetPosition(const glm::vec3& position) { _position = position; }
	void SetRotation(const glm::quat& rotation) { _rotation = rotation; }
	const glm::vec3& GetPosition() const { return _position; }
	const glm::quat& GetRotation() const { return _rotation; }
	void SetAnimation(const std::string&);

	void Update(double deltatime);
	void Draw(const glm::mat4& viewProjection, GL::ShaderProgram&, const ResourceManager&);

	// maybe change this to just anim names
	const std::unordered_map<std::string, std::unique_ptr<SkinAnimation>>& GetAnimations() const { return _animations; }

  private:
	void loadSkeleton(const P3D::Skeleton&);
	void addAnimation(const P3D::Animation&);

	glm::vec3 _position;
	glm::quat _rotation;
	SkinAnimation* _currentAnimation;

	std::unordered_map<std::string, std::unique_ptr<SkinAnimation>> _animations;
	std::unique_ptr<SkinModel> _skinModel;
	std::vector<SkeletonJoint> _skeletonJoints;
	// std::weak_ptr<SkinModel> _skinModel; // use when SkinModels are created as a shared_ptr
	std::unordered_map<std::string, std::unique_ptr<GL::Texture2D>> _textureMap;
	std::unordered_map<std::string, std::string> _shaderTextureMap;

	std::unique_ptr<GL::TextureBuffer> _boneBuffer;
	std::vector<glm::mat4> _boneMatrices;
	std::vector<glm::mat4> _poseMatrices;
	std::vector<glm::mat4> _finalMatrices;
};
} // namespace Donut
