#pragma once

#include <GL/ShaderProgram.h>

#include <ResourceManager.h>
#include <StaticEntity.h>
#include <WorldSphere.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>

namespace Donut
{

class Level
{
public:
	Level();

	void Draw(const ResourceManager&, glm::mat4& viewProj);
	void LoadP3D(const std::string& filename);

private:
	std::unique_ptr<ResourceManager> _resourceManager;
    std::unique_ptr<WorldSphere> _worldSphere;
	std::vector<std::unique_ptr<StaticEntity>> _staticEntities;
	std::unique_ptr<GL::ShaderProgram> _worldShader;
};

} // namespace Donut
