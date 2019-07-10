#pragma once

#include <glad/glad.h>
#include <map>
#include <string>

#include <glm/glm.hpp>

namespace Donut::GL {

class ShaderProgram
{
  public:
	ShaderProgram() = delete;
	ShaderProgram(const std::string& vertexSource, const std::string& fragmentSource);
	~ShaderProgram();

	void Bind();

	void SetUniformValue(const char* uniformName, int value);
	void SetUniformValue(const char* uniformName, float value);
	void SetUniformValue(const char* uniformName, const glm::vec2& v);
	void SetUniformValue(const char* uniformName, const glm::vec3& v);
	void SetUniformValue(const char* uniformName, const glm::vec4& v);
	void SetUniformValue(const char* uniformName, const glm::mat3& m);
	void SetUniformValue(const char* uniformName, const glm::mat4& m);
	void SetUniformValue(const char* uniformName, size_t count, const glm::mat4* m);

	GLuint GetRawHandle() const { return _program; }

  private:
	GLuint _program;
	std::map<std::string, GLint> _uniforms;

	GLuint createSubShader(GLenum type, const std::string& source);
};

} // namespace Donut::GL
