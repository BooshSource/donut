#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

#include <GL/Texture2D.h>

namespace Donut {

class ResourceManager {
public:
	ResourceManager();

	void AddTexture(const std::string& name, std::unique_ptr<GL::Texture2D> texture);
	const GL::Texture2D& GetTexture(const std::string& name) const;
	const std::map<std::string, std::unique_ptr<GL::Texture2D>>& GetTextures() const { return _textures; }

  protected:
	std::map<std::string, std::unique_ptr<GL::Texture2D>> _textures;
	std::unique_ptr<GL::Texture2D> _errorTexture;
};

} // namespace Donut
