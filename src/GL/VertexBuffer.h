#pragma once

#include <glad/glad.h>
#include <memory>

namespace Donut::GL
{

class VertexBuffer
{
  public:
	VertexBuffer()                          = delete;
	VertexBuffer(const VertexBuffer& other) = delete;
	VertexBuffer(VertexBuffer&&)            = default;

	VertexBuffer(const void* vertices, std::size_t vertexCount, size_t strideBytes, GLuint hint = GL_STATIC_DRAW);

	~VertexBuffer();

	VertexBuffer& operator=(const VertexBuffer&) = delete;
	VertexBuffer& operator=(VertexBuffer&&) = default;

	std::size_t GetVertexCount() const noexcept;
	std::size_t GetStrideBytes() const noexcept;
	std::size_t GetSizeInBytes() const noexcept;
	GLuint GetHint() const noexcept;
	GLuint GetVBO() const noexcept;

	//const std::unique_ptr<GLvoid*> GetData() const;

	inline void Bind() { glBindBuffer(GL_ARRAY_BUFFER, _vbo); }

  private:
	GLuint _vbo;
	std::size_t _vertexCount;
	std::size_t _strideBytes;
	GLuint _hint;
};

} // namespace Donut::GL
