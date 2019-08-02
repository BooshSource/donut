#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>

#include <memory>
#include <string>
#include <vector>

namespace Donut
{

class Window;
class ResourceManager;
class LineRenderer;
class Level;
class WorldPhysics;
class FreeCamera;
class Character;

namespace P3D
{
class P3DFile;
class TextureFont;
}

namespace RCL
{
	class RCFFile;
}

namespace GL
{
class ShaderProgram;
}

class Game
{
  public:
	Game(int argc, char** argv);
	~Game();

	Window& GetWindow() const
	{
		return *_window;
	}

	void Run();
	void LoadModel(const std::string&, const std::string&);

	ResourceManager& GetResourceManager() { return *_resourceManager; }
	WorldPhysics& GetWorldPhysics() { return *_worldPhysics; }

	void LockMouse(bool lockMouse);

	static Game& GetInstance() { return *instance; }

  private:
	void guiTeleportMenu();
	void guiModelMenu(Character&);
	void guiDebugMenu();
	void loadGlobal();
	void debugDrawP3D(const P3D::P3DFile&);
	void debugDrawRCF();
	void TestAudio();
	void PlayAudio(RCL::RCFFile& file, const std::string& filename);
	void OnInputTextEntry(const std::string& text);

	std::unique_ptr<Window> _window;
	std::unique_ptr<ResourceManager> _resourceManager;
	std::unique_ptr<FreeCamera> _camera;
	std::unique_ptr<Character> _character;
	std::unique_ptr<LineRenderer> _lineRenderer;
	std::unique_ptr<Level> _level;
	std::unique_ptr<WorldPhysics> _worldPhysics;
	std::unique_ptr<P3D::P3DFile> _animP3D;
	std::unique_ptr<P3D::P3DFile> _globalP3D;
	std::unique_ptr<P3D::TextureFont> _textureFontP3D;
	std::vector<std::unique_ptr<RCL::RCFFile>> _filesRCF;

	std::unique_ptr<Character> _npcCharacter;


	std::unique_ptr<GL::ShaderProgram> _skinShaderProgram;

	bool _mouseLocked;
	int _lockedMousePosX;
	int _lockedMousePosY;

	ALuint buffer, source;

	static Game* instance;
};

} // namespace Donut
