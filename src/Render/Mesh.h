// Copyright 2019 the donut authors. See AUTHORS.md

#pragma once

#include <Render/OpenGL/IndexBuffer.h>
#include <Render/OpenGL/ShaderProgram.h>
#include <Render/OpenGL/VertexBuffer.h>
#include <Render/OpenGL/VertexBinding.h>
#include <Render/SkinAnimation.h>
#include <P3D/p3d.generated.h>
#include <ResourceManager.h>
#include <glm/glm.hpp>
#include <string>

namespace Donut
{

class Mesh
{
	struct PrimGroup
	{
		std::string shaderName;
		GLenum type;
		std::size_t indicesOffset;
		std::size_t indicesCount;
		Shader* cacheShader;
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec4 co0lor;
	};

  public:
	Mesh(const P3D::Mesh& mesh, bool instanced = false);
	void Draw(bool opaque);

  private:
	std::string _name;
	std::vector<PrimGroup> _primGroups;

	std::unique_ptr<GL::VertexBuffer> _vertexBuffer;
	std::unique_ptr<GL::VertexBuffer> _instanceBuffer;
	std::unique_ptr<GL::IndexBuffer> _indexBuffer;
	std::unique_ptr<GL::VertexBinding> _vertexBinding;

	glm::vec3 _boundingBoxMin;
	glm::vec3 _boundingBoxMax;
};

} // namespace Donut
