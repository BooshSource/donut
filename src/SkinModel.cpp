#include <iostream>
#include <SkinModel.h>
namespace Donut {

SkinModel::SkinModel(const std::string& filename) : _filename(filename) {
    _p3dFile = std::make_unique<P3D::P3DFile>(filename);

    const auto& root = _p3dFile->GetRoot();
    for (const auto& chunk : root.GetChildren()) {
        switch (chunk->GetType()) {
        case P3D::ChunkType::PolySkin: {
            P3D::PolySkinLoader loader;
            _polySkin = loader.Load(*chunk.get());

            break;
        }
        default:
            break;
        }
    }

    createMesh();
}

std::string vertexShader = R"glsl(
	#version 150 core

	in vec3 position;
	in vec3 normal;
	in vec3 boneWeights;
	in ivec3 boneIndices;

	out vec3 Normal;

	uniform mat4 viewProj;
	uniform samplerBuffer boneBuffer;

	mat4 GetMatrix(int index)
	{
		return mat4(texelFetch(boneBuffer, (index * 4) + 0),
				    texelFetch(boneBuffer, (index * 4) + 1),
				    texelFetch(boneBuffer, (index * 4) + 2),
				    texelFetch(boneBuffer, (index * 4) + 3));
	}

	void main()
	{
		Normal = normal;
		gl_Position = viewProj * vec4(position, 1.0);
	}
)glsl";

std::string fragmentShader = R"glsl(
	#version 150 core

	in vec3 Normal;
	out vec4 outColor;

	void main()
	{
		vec3 rgb_normal = Normal * 0.5 + 0.5;
		outColor = vec4(rgb_normal, 1.0);
	}
)glsl";

void SkinModel::createMesh() {
    _shader = std::make_unique<GL::ShaderProgram>(vertexShader, fragmentShader);

    std::vector<Vertex> allVerts;
    std::vector<std::uint32_t> allIndices;

    std::uint32_t vertOffset = 0;
    for (auto const& prim : _polySkin->GetPrimGroups()) {
        auto verts = prim->GetVerticies();
        auto normals = prim->GetNormals();
        auto indices = prim->GetIndices();

        for (std::uint32_t i = 0; i < verts.size(); i++) {
            allVerts.push_back(Vertex{verts[i], normals[i]});
        }

        for (std::uint32_t i = 0; i < indices.size(); i++) {
            allIndices.push_back(indices[i] + vertOffset);
        }

        vertOffset += verts.size();
    }

    glGenVertexArrays(1, &_vertexArrayObject);
    glBindVertexArray(_vertexArrayObject);

    _vertexBuffer =
        std::make_unique<GL::VertexBuffer>(allVerts.data(), allVerts.size(), sizeof(Vertex));
    _indexBuffer =
        std::make_unique<GL::IndexBuffer>(allIndices.data(), allIndices.size(), GL_UNSIGNED_INT);

	int ptr = 0;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)ptr);
    ptr += sizeof(glm::vec3);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)ptr);
    ptr += sizeof(glm::vec3);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)ptr);
    ptr += sizeof(glm::vec3);
    glVertexAttribIPointer(3, 3, GL_INT, sizeof(Vertex), (GLvoid*)ptr);
    ptr += sizeof(glm::ivec3);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

void SkinModel::Draw(glm::mat4& viewProj) {
    _shader->Bind();
    _shader->SetUniformValue("viewProj", viewProj);
    //_shader->SetUniformValue("boneBuffer", nullptr); // TODO

    glBindVertexArray(_vertexArrayObject);

    std::uint32_t idxOffset = 0;
    for (auto const& prim : _polySkin->GetPrimGroups()) {
        auto indicesSize = prim->GetIndices().size();

        GLenum mode = GL_TRIANGLE_STRIP;
		switch (prim->GetPrimitiveType()) {
		case P3D::PrimGroup::PrimitiveType::TriangleStrip:
            mode = GL_TRIANGLE_STRIP;
            break;
        case P3D::PrimGroup::PrimitiveType::TriangleList:
            mode = GL_TRIANGLES;
            break;
        case P3D::PrimGroup::PrimitiveType::LineStrip:
            mode = GL_LINE_STRIP;
            break;
        case P3D::PrimGroup::PrimitiveType::LineList:
            mode = GL_LINES;
            break;
		}

        glDrawElements(mode, indicesSize, _indexBuffer->GetType(), (void*)(idxOffset * 4));
        idxOffset += indicesSize;
    }

    glBindVertexArray(0);
}

} // namespace Donut
