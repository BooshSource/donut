#include <Level.h>
#include <P3D/P3DFile.h>
#include <P3D/StaticEntity.h>
#include <P3D/StaticPhys.h>
#include <P3D/Texture.h>
#include <P3D/WorldSphere.h>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "P3D/Intersect.h"

namespace Donut
{

std::string lvlVertexShader = R"glsl(
	#version 150 core

	in vec3 position;
	in vec2 uv;
	in vec4 color;

	out vec2 texCoord;
	out vec4 vertColor;

	uniform mat4 viewProj;

	void main()
	{
		texCoord = uv;
		vertColor = color;
		gl_Position = viewProj * vec4(position, 1.0);
	}
)glsl";

std::string lvlFragmentShader = R"glsl(
	#version 150 core

	uniform sampler2D diffuseTex;

	in vec2 texCoord;
	in vec4 vertColor;

	out vec4 outColor;

	void main()
	{
		vec4 diffuseColor = texture2D(diffuseTex, texCoord);
		if (diffuseColor.a == 0.0)
			discard;

        outColor = vec4(diffuseColor.rgb * vertColor.rgb, diffuseColor.a);
	}
)glsl";

Level::Level()
{
	_worldShader     = std::make_unique<GL::ShaderProgram>(lvlVertexShader, lvlFragmentShader);
	_resourceManager = std::make_unique<ResourceManager>();
}

void Level::LoadP3D(const std::string& filename)
{
	if (!std::filesystem::exists(filename))
	{
		std::cout << "Level not found: " << filename << "\n";
		return;
	}

	std::cout << "Loading level: " << filename << "\n";

	const auto p3d = P3D::P3DFile(filename);

	const auto& root = p3d.GetRoot();
	for (const auto& chunk : root.GetChildren())
	{
		switch (chunk->GetType())
		{
		case P3D::ChunkType::Shader:
		{
			auto shader                   = P3D::Shader::Load(*chunk);
			const std::string shader_name = shader->GetName();
			_resourceManager->AddShader(shader_name, std::move(shader));
			break;
		}
		case P3D::ChunkType::Texture:
		{
			auto texture = P3D::Texture::Load(*chunk);
			auto texdata = texture->GetData();

			std::unique_ptr<GL::Texture2D> tex;
			if (texdata.comp == 4)
				tex = std::make_unique<GL::Texture2D>(texdata.width, texdata.height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, texdata.data.data());
			else
				tex = std::make_unique<GL::Texture2D>(texdata.width, texdata.height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, texdata.data.data());

			_resourceManager->AddTexture(texture->GetName(), std::move(tex));

			break;
		}
		case P3D::ChunkType::StaticEntity:
		{
			const auto& ent = P3D::StaticEntity::Load(*chunk);
			auto model      = std::make_unique<StaticEntity>(*ent);

			_staticEntities.push_back(std::move(model));
			break;
		}
		case P3D::ChunkType::StaticPhys:
		{
			const auto& ent = P3D::StaticPhys::Load(*chunk);
			// std::cout << "StaticPhys: " << ent->GetName() << "\n";
			// std::cout << "\tCollisionObject: " << ent->GetCollisionObject().GetName() << "\n";

			//auto model      = std::make_unique<StaticEntity>(*ent);

			//_staticEntities.push_back(std::move(model));
			break;
		}
		case P3D::ChunkType::Intersect:
		{
			auto intersect = P3D::Intersect::Load(*chunk);
			_intersects.push_back(std::move(intersect));
			break;
		}
		case P3D::ChunkType::WorldSphere:
		{
			auto worldSphere = P3D::WorldSphere::Load(*chunk);
			std::cout << "world sphere: " << worldSphere->GetName() << " has " << worldSphere->GetMeshes().size() << " meshes\n";
			_worldSphere = std::make_unique<WorldSphere>(*worldSphere);
			break;
		}
		default: break;
		}
	}
}

void Level::Draw(const ResourceManager& rm, glm::mat4& viewProj)
{
	_worldShader->Bind();
	_worldShader->SetUniformValue("viewProj", viewProj);

	if (_worldSphere != nullptr)
		_worldSphere->Draw(*_worldShader, *_resourceManager);

	for (const auto& ent : _staticEntities)
	{
		ent->Draw(*_worldShader, *_resourceManager);
	}
}

} // namespace Donut
