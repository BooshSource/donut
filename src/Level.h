#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <Render/WorldSphere.h>
#include <Render/CompositeModel.h>
#include <Entity.h>

namespace Donut
{

namespace GL
{
class ShaderProgram;
}

class ResourceManager;


class Level
{
public:
	Level();

	void Draw(glm::mat4& viewProj);
	void LoadP3D(const std::string& filename);

	void DynaLoadData(const std::string& dynaLoadData);

	void ImGuiDebugWindow(bool* p_open) const;

  private:
	void loadRegion(const std::string& filename);
	void unloadRegion(const std::string& filename);

    std::unique_ptr<WorldSphere> _worldSphere;
	std::vector<std::unique_ptr<Entity>> _entities;
	std::unique_ptr<GL::ShaderProgram> _worldShader;

	std::vector<std::unique_ptr<CompositeModel>> _compositeModels;
};

} // namespace Donut
