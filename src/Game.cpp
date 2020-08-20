// Copyright 2019-2020 the donut authors. See AUTHORS.md

#include "Game.h"

#include "Audio/AudioManager.h"
#include "Core/Log.h"
#include "Core/FpsTimer.h"
#include "Core/Math/Math.h"
#include "FreeCamera.h"
#include "Input/Input.h"
#include "Physics/WorldPhysics.h"
#include "Pure3D/ChunkFile.h"
#include "Pure3D/P3DFileHandler.h"
#include "RCL/RCFFile.h"
#include "RCL/RSDFile.h"
#include "Render/LineRenderer.h"
#include "Render/OpenGL/FrameBuffer.h"
#include "Render/OpenGL/ShaderProgram.h"
#include "Render/OpenGL/glad/gl.h"
#include "Render/Shader.h"
#include "Render/imgui/imgui.h"
#include "Render/imgui/imgui_impl_opengl3.h"
#include "Render/imgui/imgui_impl_sdl.h"
#include "Render/imgui/imgui_markdown.h"
#include "ResourceManager.h"
#include "Scripting/Commands.h"
#include "Window.h"

#include <SDL.h>
#include <fmt/format.h>

#include <array>
#include <iostream>
#include <sstream>
#include <string>

namespace Donut
{

Game* Game::instance = nullptr;

#if _DEBUG
const std::string kBuildString = "DEBUG BUILD";
#else
const std::string kBuildString = "Release Build";
#endif

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                                const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

Game::Game(int argc, char** argv)
{
	Log::Info("donut - simpsons\nbuild date: {}\n\n", __DATE__);

	instance = this; // global static :D

	Commands::RunLine("HelloWorld();");

	// for (const auto& entry : FileSystem::recursive_directory_iterator("scripts"))
	//{
	//	const auto& path = entry.path();
	//	const auto& extension = path.extension().string();
	//	if (extension != ".con" && extension != ".mfk") continue;

	//	Commands::RunScript(path.string());
	//}

	const std::string windowTitle = fmt::format("donut [{0}]", kBuildString);

	const int windowWidth = 1280, windowHeight = 960;
	_window = std::make_unique<Window>(windowTitle, windowWidth, windowHeight);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MessageCallback, nullptr);
	// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, GL_FALSE);

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(static_cast<SDL_Window*>(*_window), static_cast<SDL_GLContext*>(*_window));
	ImGui_ImplOpenGL3_Init("#version 130");

	// const float dpi_scale = 2.0f;
	// ImGuiIO& io = ImGui::GetIO();
	// ImGui::GetStyle().ScaleAllSizes(dpi_scale);
	// io.FontGlobalScale = dpi_scale;

	_lineRenderer = std::make_unique<LineRenderer>(1000000);
	_worldPhysics = std::make_unique<WorldPhysics>(_lineRenderer.get());

	// init sub classes
	_audioManager = std::make_unique<AudioManager>();
	_resourceManager = std::make_unique<ResourceManager>();

	// todo: load art\frontend\dynaload\images\mouse_cursor.p3d and use mouse_cursor.png Sprite
	
	const auto skinVertSrc = File::ReadAll("shaders/skin.vert");
	const auto skinFragSrc = File::ReadAll("shaders/skin.frag");
	_skinShaderProgram = std::make_unique<GL::ShaderProgram>(skinVertSrc, skinFragSrc);

	_camera = std::make_unique<FreeCamera>();
	_camera->SetPosition(Vector3(228.0f, 5.0f, -174.0f));
	_camera->SetFOV(70.0f);
	_camera->SetZNear(1.0f);
	_camera->SetZFar(100000.0f);

	_mouseLocked = false;
}

Game::~Game()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	// todo: might need to reset our unique_ptrs here in a certain order...

	_window.reset();

	SDL_Quit();
}

void Game::InitializePure3D()
{
	P3DFileHandler handler;
	// handler.AddHandler(TextureLoader);
}

void Game::LoadP3D(const std::string& filename)
{
	if (!FileSystem::exists(filename)) {
		Log::Error("P3D not found: {}\n", filename);
		return;
	}

	Log::Info("Loading P3D: {}\n", filename);

	File file(filename, FileMode::Read);
	P3DFileHandler handler;
	handler.Load(&file);

	// handler.AddHandler(TextureLoader)

}

void Game::LockMouse(bool lockMouse)
{
	if (_mouseLocked == lockMouse)
		return;

	_mouseLocked = lockMouse;

	SDL_SetRelativeMouseMode(lockMouse ? SDL_TRUE : SDL_FALSE);

	if (lockMouse)
	{
		SDL_GetMouseState(&_lockedMousePosX, &_lockedMousePosY);
	}

	int w, h;
	SDL_GetWindowSize(static_cast<SDL_Window*>(*_window), &w, &h);
	SDL_WarpMouseInWindow(static_cast<SDL_Window*>(*_window), w / 2, h / 2);

	Input::ResetMouseDelta();
}

std::vector<std::tuple<std::string, Vector3, std::string>> locations {
    {"Simpsons' House", Vector3(220, 3.5, -172), "l1z1.p3d;l1r1.p3d;l1r7.p3d;"},
    {"Kwik E Mart", Vector3(209, 3.6, -285), "l1z2.p3d;l1r1.p3d;l1r2.p3d;"},
    {"Church", Vector3(193.8, -0.9, -570), "l1r2.p3d;l1z2.p3d;l1z3.p3d;"},
    {"Springfield Elementary", Vector3(-11, 0.7, -586), "l1z3.p3d;l1r2.p3d;l1r3.p3d;"},
    {"Burns' Mansion", Vector3(-186, 3.5, -96), "l1z4.p3d;l1r3.p3d;l1r4a.p3d;"},
    {"Stonecutters Tunnel", Vector3(-405, 2, 60), "l1z4.p3d;l1r3.p3d;l1r4a.p3d;"},
    {"Power Plant Interior", Vector3(-80, 0.8, 297), "l1r4a.p3d;l1z6.p3d;l1r6.p3d;"},
    {"Power Plant Parking Lot", Vector3(40, 0, 296), "l1z6.p3d;l1r6.p3d;"},
    {"Tomacco", Vector3(190, -0.7, 425), "l1r6.p3d;l1z6.p3d;l1z7.p3d;"},
    {"Trailer Park", Vector3(391, -2.2, 494), "l1z7.p3d;l1r6.p3d;l1r7.p3d;"},
    {"Cletus' House", Vector3(333.5, -1.8, 356), "l1z7.p3d;l1r6.p3d;l1r7.p3d;"},
    {"Graveyard", Vector3(368, 5.1, 5.4), "l1z1.p3d;l1r1.p3d;l1r7.p3d;"}};

std::vector<std::pair<std::string, std::string>> models {
    {"homer", "homer"}, {"h_evil", "homer"}, {"h_fat", "homer"}, {"h_undr", "homer"},
    {"marge", "marge"}, {"bart", "bart"},    {"apu", "apu"},     {"a_amer", "apu"},
};

void Game::OnInputTextEntry(const std::string& text) {}

void Game::Run()
{
	// measure our delta time
	uint64_t now = SDL_GetPerformanceCounter();
	uint64_t last = 0;
	double deltaTime = 0.0;

	FpsTimer timer;

	Input::CaptureTextEntry(this, &Game::OnInputTextEntry);

	LoadP3D("./art/l1z1.p3d");

	SDL_Event event;
	bool running = true;
	while (running)
	{
		last = now;
		now = SDL_GetPerformanceCounter();

		deltaTime = ((now - last) / (double)SDL_GetPerformanceFrequency());
		timer.Update(deltaTime);

		Input::PreEvent();

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;

			ImGui_ImplSDL2_ProcessEvent(&event);

			Input::HandleEvent(event);
		}

		LockMouse(Input::IsDown(Button::MouseRight));

		if (_mouseLocked)
		{
			const float mouseDeltaX = Input::GetMouseDeltaX();
			const float mouseDeltaY = Input::GetMouseDeltaY();

			_camera->LookDelta(mouseDeltaX * 0.25f, mouseDeltaY * 0.25f);
		}

		auto inputForce = Vector3(0.0f);
		if (Input::IsDown(Button::KeyW))
			inputForce += Vector3::Forward;
		if (Input::IsDown(Button::KeyS))
			inputForce += Vector3::Backward;
		if (Input::IsDown(Button::KeyA))
			inputForce += Vector3::Left;
		if (Input::IsDown(Button::KeyD))
			inputForce += Vector3::Right;
		if (Input::IsDown(Button::KeySPACE))
			inputForce += Vector3::Up;
		if (Input::IsDown(Button::KeyLCONTROL))
			inputForce += Vector3::Down;
		if (inputForce.LengthSquared() > 0.0f)
		{
			inputForce.Normalize();
			inputForce *= Input::IsDown(Button::KeyLSHIFT) ? 60.0f : 10.0f;
			_camera->Move(inputForce, static_cast<float>(deltaTime));
		}

		_worldPhysics->Update(static_cast<float>(deltaTime));

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(static_cast<SDL_Window*>(*_window));
		ImGui::NewFrame();

		imguiMenuBar();

		// if (_character != nullptr)
		//	guiModelMenu(*_character);

		// ImGui::ShowDemoWindow();

		if (_debugResourceManagerWindowOpen)
			_resourceManager->ImGuiDebugWindow(&_debugResourceManagerWindowOpen);

		if (_debugAudioWindowOpen)
			_audioManager->DebugGUI(&_debugAudioWindowOpen);

		debugAboutMenu();

		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 8.0f, io.DisplaySize.y - 8.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
		ImGui::SetNextWindowBgAlpha(0.35f);
		if (ImGui::Begin("Camera position overlay", NULL,
		                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
		                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
		{
			auto const& camPos = _camera->GetPosition();
			auto const& camRot = _camera->GetOrientation();
			ImGui::Text("Camera Position: %s", camPos.ToString().c_str());
			ImGui::Text("Camera Orientation: %s", camRot.ToString().c_str());

			float fov = _camera->GetFOV();
			if (ImGui::SliderFloat("FOV", &fov, 0.0f, 120.0f))
				_camera->SetFOV(fov);
		}
		ImGui::End();

		ImGui::Render();

		int viewportWidth = 0;
		int viewportHeight = 0;

		viewportWidth = (int)io.DisplaySize.x;
		viewportHeight = (int)io.DisplaySize.y;

		glViewport(0, 0, viewportWidth, viewportHeight);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		_camera->SetAspectRatio(static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight));

		Matrix4x4 viewMatrix = _camera->GetViewMatrix();
		Matrix4x4 projMatrix = _camera->GetProjectionMatrix();
		Matrix4x4 viewProjection = projMatrix * viewMatrix;

		glDisable(GL_DEPTH_TEST);
		_lineRenderer->Flush(viewProjection);
		glEnable(GL_DEPTH_TEST);

		Matrix4x4 proj = Matrix4x4::MakeOrtho(0.0f, viewportWidth, viewportHeight, 0.0f);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		_window->Swap();
	}
}

void Game::imguiMenuBar()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("Warp"))
	{
		// if (ImGui::BeginMenu("Level 1"))
		// {
		for (auto const& location : locations)
		{
			if (ImGui::MenuItem(std::get<0>(location).c_str()))
			{
				const Vector3& dest = std::get<1>(location);
				//_worldPhysics->GetCharacterController()->SetPosition(dest);
				_camera->SetPosition(dest);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(std::get<1>(location).ToString().c_str());
				ImGui::TextUnformatted(std::get<2>(location).c_str());
				ImGui::EndTooltip();
			}
		}
		//	ImGui::EndMenu();
		//}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Physics"))
	{
		PhysicsDebugDrawMode mode = _worldPhysics->GetDebugDrawMode();

		ImGui::CheckboxFlags("Draw Wireframe", reinterpret_cast<unsigned int*>(&mode),
		                     static_cast<unsigned int>(PhysicsDebugDrawMode::DrawWireframe));
		ImGui::CheckboxFlags("Draw AABB", reinterpret_cast<unsigned int*>(&mode),
		                     static_cast<unsigned int>(PhysicsDebugDrawMode::DrawAABB));
		ImGui::CheckboxFlags("Draw Features Text", reinterpret_cast<unsigned int*>(&mode),
		                     static_cast<unsigned int>(PhysicsDebugDrawMode::DrawFeaturesText));
		ImGui::CheckboxFlags("Draw Contact Points", reinterpret_cast<unsigned int*>(&mode),
		                     static_cast<unsigned int>(PhysicsDebugDrawMode::DrawContactPoints));
		ImGui::CheckboxFlags("Draw Text", reinterpret_cast<unsigned int*>(&mode),
		                     static_cast<unsigned int>(PhysicsDebugDrawMode::DrawText));
		ImGui::CheckboxFlags("Fast Wireframe", reinterpret_cast<unsigned int*>(&mode),
		                     static_cast<unsigned int>(PhysicsDebugDrawMode::FastWireframe));
		ImGui::CheckboxFlags("Draw Normals", reinterpret_cast<unsigned int*>(&mode),
		                     static_cast<unsigned int>(PhysicsDebugDrawMode::DrawNormals));
		ImGui::CheckboxFlags("Draw Frames", reinterpret_cast<unsigned int*>(&mode),
		                     static_cast<unsigned int>(PhysicsDebugDrawMode::DrawFrames));

		_worldPhysics->SetDebugDrawMode(mode);

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Audio"))
		_debugAudioWindowOpen = true;

	if (ImGui::MenuItem("Resource Manager"))
		_debugResourceManagerWindowOpen = true;

	if (ImGui::MenuItem("About"))
		_debugAboutWindowOpen = true;

	ImGui::EndMainMenuBar();
}

const std::string aboutText =
    R"(donut - copyright 2019-2020 by [Matt Stevens](https://github.com/handsomematt), [Michael Johnson](https://github.com/aylaylay)
# License
donut is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

donut is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see [<https://www.gnu.org/licenses/>](https://www.gnu.org/licenses/).
# Open Source Software
This software uses open source components, whose copyright and other rights belong to their respective owners:

* sdl2 [https://www.libsdl.org/](https://www.libsdl.org/)
* glad [https://github.com/Dav1dde/glad](https://github.com/Dav1dde/glad)
* bullet [https://github.com/bulletphysics/bullet3](https://github.com/bulletphysics/bullet3)
* openal-soft [https://github.com/kcat/openal-soft](https://github.com/kcat/openal-soft)
* dear imgui [https://github.com/ocornut/imgui](https://github.com/ocornut/imgui)
* imgui_markdown [https://github.com/juliettef/imgui_markdown](https://github.com/juliettef/imgui_markdown)
* fmt [https://github.com/fmtlib/fmt/](https://github.com/fmtlib/fmt/)
* stb [https://github.com/nothings/stb](https://github.com/nothings/stb)
)";

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#endif

void LinkCallback(ImGui::MarkdownLinkCallbackData data_)
{
	std::string url(data_.link, data_.linkLength);
	if (!data_.isImage)
	{
#ifdef _WIN32
		ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
		std::string xdgOpenURL = "xdg-open " + url;
		system(xdgOpenURL.c_str());
#endif
	}
}

static ImGui::MarkdownConfig mdConfig {LinkCallback};

void Game::debugAboutMenu()
{
	if (!_debugAboutWindowOpen)
		return;

	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	const ImVec2 center(displaySize.x * 0.5f, displaySize.y * 0.5f);
	const ImVec2 windowSize(600.0f, 520.0f);

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);
	ImGui::SetNextWindowPos(center, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
	if (ImGui::Begin("About", &_debugAboutWindowOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Markdown(aboutText.c_str(), aboutText.length(), mdConfig);
	}
	ImGui::End();
}

} // namespace Donut
