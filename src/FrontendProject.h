// Copyright 2019 the donut authors. See AUTHORS.md

#pragma once

#include <Render/SpriteBatch.h>
#include <glad/glad.h>
#include <memory>
#include <string>

namespace Donut
{
class ResourceManager;
class Texture;

namespace P3D
{
class FrontendGroup;
class FrontendMultiSprite;
} // namespace P3D

class FrontendProject
{
  public:
	FrontendProject();
	~FrontendProject();

	void LoadP3D(const std::string&);
	void Draw(const glm::mat4& proj);

  private:
	void AddGroup(const P3D::FrontendGroup& group, int32_t resX, int32_t resY);
	void AddMultiSprite(const P3D::FrontendMultiSprite& multiSprite, int32_t resX, int32_t resY);

	struct Sprite
	{
		Texture* texture;
		int32_t positionX;
		int32_t positionY;
		int32_t width;
		int32_t height;
		glm::vec4 color;
	};

	SpriteBatch _spriteBatch;
	std::vector<Sprite> _sprites;

	GLuint _sampler;
};
} // namespace Donut
