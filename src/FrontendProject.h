// Copyright 2019 the donut authors. See AUTHORS.md

#pragma once

#include <string>
#include <memory>
#include <Render/SpriteBatch.h>

namespace Donut
{
	class ResourceManager;

	namespace GL
	{
		class Texture2D;
	}

	class FrontendProject
	{
	public:
		void LoadP3D(const std::string&);
		void Draw(const glm::mat4& proj);

	private:

		std::vector<std::unique_ptr<GL::Texture2D>> _textures;
		SpriteBatch _spriteBatch;
	};
}
