#include <Character.h>
#include <Core/FpsTimer.h>
#include <FreeCamera.h>
#include <Game.h>
#include <Input/Input.h>
#include <Level.h>
#include <P3D/P3DFile.h>
#include <P3D/Texture.h>
#include <P3D/TextureFont.h>
#include <RCL/RCFFile.h>
#include <Physics/WorldPhysics.h>
#include <Render/LineRenderer.h>
#include <Render/OpenGL/ShaderProgram.h>
#include <Render/SkinModel.h>
#include <Render/SpriteBatch.h>
#include <ResourceManager.h>
#include <SDL.h>
#include <Window.h>
#include <fmt/format.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <array>

namespace Donut
{

#if _DEBUG
const std::string kBuildString = "DEBUG BUILD";
#else
const std::string kBuildString = "Release Build";
#endif

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

void Game::TestAudio()
{
	// THIS IS ALL SHIT, PROOF OF CONCEPT!!

	if (_filesRCF.empty()) return;

	ALCdevice* device;
	ALCcontext* context;

	device = alcOpenDevice(NULL);
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);

	alGenBuffers(1, &buffer);
	alGenSources(1, &source);

	PlayAudio(*_filesRCF[1], "sound\\music\\muzak.rsd");

		//auto magic = rsdStream->ReadString(8);
		//auto numChannels = rsdStream->Read<uint32_t>();
		//auto bitsPerChannel = rsdStream->Read<uint32_t>();
		//auto sampleRate = rsdStream->Read<uint32_t>();

		//rsdStream->Seek(0x800, Donut::SeekMode::Begin);
		//std::vector<uint8_t> data(rsdStream->Size() - rsdStream->Position());
		//rsdStream->ReadBytes(data.data(), data.size());



		//alBufferData(buffer, AL_FORMAT_STEREO16, data.data(), (ALsizei)data.size(), sampleRate);
		//alGenSources(1, &source);
		//alSourcei(source, AL_BUFFER, buffer);
		//alSourcei(source, AL_LOOPING, AL_TRUE);
		//alSourcePlay(source);

		//alSourceStop(source);
		//alDeleteSources(1, &source);
		//alDeleteBuffers(1, &buffer);
		//alcMakeContextCurrent(NULL);
		//alcDestroyContext(context);
		//alcCloseDevice(device);
}

std::array<int, 16> indexTable = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
std::array<int, 89> stepTable =
{
	7, 8, 9, 10, 11, 12, 13, 14,
	16, 17, 19, 21, 23, 25, 28, 31,
	34, 37, 41, 45, 50, 55, 60, 66,
	73, 80, 88, 97, 107, 118, 130, 143,
	157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411,
	1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767
};

static void DecodeNibble(int32_t nibble, int32_t& nibbleDecoded, int32_t& stepIndex)
{
	int32_t step = stepTable[stepIndex];
	int32_t delta = step >> 3;
	if (nibble & 1) delta += step >> 2;
	if (nibble & 2) delta += step >> 1;
	if (nibble & 4) delta += step;
	if (nibble & 8) delta = -delta;

	nibbleDecoded += delta;
	if (nibbleDecoded > 32767) nibbleDecoded = 32767;
	else if (nibbleDecoded < -32768) nibbleDecoded = -32768;

	stepIndex += indexTable[nibble];
	if (stepIndex < 0) stepIndex = 0;
	else if (stepIndex > 88) stepIndex = 88;
}

static void DecodeRADP(MemoryStream& stream, int16_t* outBuffer, int32_t numBlocks, int32_t numChannels)
{
	const size_t blockSize = numChannels * 20;
	const size_t numBlockSamples = 32;
	size_t offset = 0x800;
	size_t firstSample = 0;

	for (size_t block = 0; block < numBlocks; ++block, firstSample += numBlockSamples)
	{
		for (size_t channel = 0; channel < numChannels; ++channel)
		{
			stream.Seek(offset + (channel * 4), Donut::SeekMode::Begin);
			int32_t stepIndex = (int32_t)stream.Read<int16_t>();
			int32_t nibbleDecoded = (int32_t)stream.Read<int16_t>();

			if (stepIndex < 0) stepIndex = 0;
			else if (stepIndex > 88) stepIndex = 88;

			for (size_t i = 0; i < numBlockSamples; ++i)
			{
				size_t byteOffset = offset + 4 * numChannels + channel + i / 2 * numChannels;
				stream.Seek(byteOffset, Donut::SeekMode::Begin);
				int8_t byte = stream.Read<int8_t>();
				int32_t nibble = (byte >> (i & 1 ? 4 : 0)) & 0xF;
				DecodeNibble(nibble, nibbleDecoded, stepIndex);
				outBuffer[((firstSample + i) * numChannels) + channel] = (int16_t)nibbleDecoded;
			}

			offset += blockSize;
		}
	}
}

void Game::PlayAudio(RCL::RCFFile& file, const std::string& filename)
{
	alSourceStop(source);

	auto rsdStream = file.GetFileStream(filename);
	if (rsdStream == nullptr) return;

	auto magic = rsdStream->ReadString(8);
	auto numChannels = rsdStream->Read<uint32_t>();
	auto bitsPerChannel = rsdStream->Read<uint32_t>();
	auto sampleRate = rsdStream->Read<uint32_t>();

	std::cout << fmt::format("{0}, channels:{1}, bpc:{2}, samplerate:{3}", magic, numChannels, bitsPerChannel, sampleRate) << std::endl;

	rsdStream->Seek(0x800, Donut::SeekMode::Begin);
	std::vector<uint8_t> data;

	if (magic == "RSD4PCM ")
	{
		data.resize(rsdStream->Size() - rsdStream->Position());
		rsdStream->ReadBytes(data.data(), data.size());
	}
	else if (magic == "RSD4RADP")
	{
		uint32_t encodedLength = rsdStream->Size() - rsdStream->Position();
		uint32_t encodedBlockSize = numChannels * 20;
		uint32_t numBlocks = encodedLength / encodedBlockSize;
		uint32_t numSamples = (numBlocks / numChannels) * 32;

		data.resize(numSamples * sizeof(int16_t));

		DecodeRADP(*rsdStream, (int16_t*)data.data(), numBlocks, numChannels);
	}

	if (buffer != 0)
	{
		alDeleteBuffers(1, &buffer);
	}

	alGenBuffers(1, &buffer);

	ALenum format = AL_FORMAT_STEREO16;
	if (numChannels == 1)
	{
		if (bitsPerChannel == 8) format = AL_FORMAT_MONO8;
		else if (bitsPerChannel == 16) format = AL_FORMAT_MONO16;
	}
	else if (numChannels == 2)
	{
		if (bitsPerChannel == 8) format = AL_FORMAT_STEREO8;
		else if (bitsPerChannel == 16) format = AL_FORMAT_STEREO16;
	}

	alBufferData(buffer, format, data.data(), (ALsizei)data.size(), sampleRate);
	alSourcei(source, AL_BUFFER, buffer);
	alSourcei(source, AL_LOOPING, AL_TRUE);
	alSourcePlay(source);
}

Game::Game(int argc, char** argv)
{
	const std::string windowTitle = fmt::format("donut [{0}]", kBuildString);

	const int windowWidth = 1280, windowHeight = 1024;
	_window = std::make_unique<Window>(windowTitle, windowWidth, windowHeight);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0,
	                      GL_FALSE);

	glEnable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(static_cast<SDL_Window*>(*_window),
	                             static_cast<SDL_GLContext*>(*_window));
	ImGui_ImplOpenGL3_Init("#version 130");


	std::vector<std::string> rcfFiles
	{
		"music00.rcf",
		"music01.rcf",
		"music02.rcf",
		"music03.rcf",
		"ambience.rcf",
		"carsound.rcf",
		"dialog.rcf",
		"nis.rcf",
		"scripts.rcf",
		"soundfx.rcf",
	};

	for (const std::string& filename : rcfFiles)
	{
		if (!std::filesystem::exists(filename)) continue;
		_filesRCF.push_back(std::make_unique<RCL::RCFFile>(filename));
	}

	TestAudio();


	// init sub classes
	_resourceManager = std::make_unique<ResourceManager>();

	const auto skinVertSrc = File::ReadAll("shaders/skin.vert");
	const auto skinFragSrc = File::ReadAll("shaders/skin.frag");
	_skinShaderProgram     = std::make_unique<GL::ShaderProgram>(skinVertSrc, skinFragSrc);

	loadGlobal();
	LoadModel("homer", "homer");

	_npcCharacter = std::make_unique<Character>("npc");
	_npcCharacter->LoadModel("marge");
	_npcCharacter->LoadAnimations("marge");
	_npcCharacter->SetPosition(glm::vec3(222.5, 4, -172));

	if (std::filesystem::exists("font0_16.p3d"))
	{
		const P3D::P3DFile p3dFont("font0_16.p3d");
		_textureFontP3D = P3D::TextureFont::Load(*p3dFont.GetRoot().GetChildren().at(0));
	}

	_lineRenderer = std::make_unique<LineRenderer>(1000000);

	_worldPhysics = std::make_unique<WorldPhysics>(_lineRenderer.get());
	_level        = std::make_unique<Level>(_worldPhysics.get());

	_level->LoadP3D("L1_TERRA.p3d");

	// simpsons house l1z1.p3d;l1r1.p3d;l1r7.p3d;
	_level->LoadP3D("l1z1.p3d");
	_level->LoadP3D("l1r1.p3d");
	_level->LoadP3D("l1r7.p3d");

	// rest of the shit, load the whole world why not!!
	_level->LoadP3D("l1r2.p3d");
	_level->LoadP3D("l1r3.p3d");
	_level->LoadP3D("l1r4a.p3d");
	_level->LoadP3D("l1r6.p3d");
	_level->LoadP3D("l1z2.p3d");
	_level->LoadP3D("l1z3.p3d");
	_level->LoadP3D("l1z4.p3d");
	_level->LoadP3D("l1z6.p3d");
	_level->LoadP3D("l1z7.p3d");

	_camera = std::make_unique<FreeCamera>();
	_camera->MoveTo(glm::vec3(228.0f, 5.0f, -174.0f));

	_mouseLocked = false;
}

Game::~Game()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	_window.reset();

	SDL_Quit();
}

void Game::loadGlobal()
{
	_globalP3D = std::make_unique<P3D::P3DFile>("global.p3d");

	const auto& root = _globalP3D->GetRoot();
	for (const auto& chunk : root.GetChildren())
	{
		if (chunk->GetType() != P3D::ChunkType::Texture) continue;

		auto texture = P3D::Texture::Load(*chunk);
		auto texdata = texture->GetData();

		auto tex2d = std::make_unique<GL::Texture2D>(texdata.width, texdata.height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, texdata.data.data());
		_resourceManager->AddTexture(texture->GetName(), std::move(tex2d));
	}
}

void Game::LoadModel(const std::string& name, const std::string& anim)
{
	if (_character != nullptr) _character.reset();

	_character = std::make_unique<Character>("pc");
	_character->LoadModel(name);
	_character->LoadAnimations(anim);
	_character->SetPosition(glm::vec3(220, 4, -172));
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

std::vector<std::tuple<std::string, glm::vec3, std::string>> locations {
	{ "Simpsons' House", glm::vec3(220, 3.5, -172), "l1z1.p3d;l1r1.p3d;l1r7.p3d;" },
	{ "Kwik E Mart", glm::vec3(209, 3.6, -285), "l1z2.p3d;l1r1.p3d;l1r2.p3d;" },
	{ "Church", glm::vec3(193.8, -0.9, -570), "l1r2.p3d;l1z2.p3d;l1z3.p3d;" },
	{ "Springfield Elementary", glm::vec3(-11, 0.7, -586), "l1z3.p3d;l1r2.p3d;l1r3.p3d;" },
	{ "Burns' Mansion", glm::vec3(-186, 3.5, -96), "l1z4.p3d;l1r3.p3d;l1r4a.p3d;" },
	{ "Stonecutters Tunnel", glm::vec3(-405, 2, 60), "l1z4.p3d;l1r3.p3d;l1r4a.p3d;" },
	{ "Power Plant Interior", glm::vec3(-80, 0.8, 297), "l1r4a.p3d;l1z6.p3d;l1r6.p3d;" },
	{ "Power Plant Parking Lot", glm::vec3(40, 0, 296), "l1z6.p3d;l1r6.p3d;" },
	{ "Tomacco", glm::vec3(190, -0.7, 425), "l1r6.p3d;l1z6.p3d;l1z7.p3d;" },
	{ "Trailer Park", glm::vec3(391, -2.2, 494), "l1z7.p3d;l1r6.p3d;l1r7.p3d;" },
	{ "Cletus' House", glm::vec3(333.5, -1.8, 356), "l1z7.p3d;l1r6.p3d;l1r7.p3d;" },
	{ "Graveyard", glm::vec3(368, 5.1, 5.4), "l1z1.p3d;l1r1.p3d;l1r7.p3d;" }
};

std::vector<std::pair<std::string, std::string>> models {
	{ "homer", "homer" },
	{ "h_evil", "homer" },
	{ "h_fat", "homer" },
	{ "h_undr", "homer" },
	{ "marge", "marge" },
	{ "bart", "bart" },
	{ "apu", "apu" },
	{ "a_amer", "apu" },
};

void Game::Run()
{
	// measure our delta time
	uint64_t now     = SDL_GetPerformanceCounter();
	uint64_t last    = 0;
	double deltaTime = 0.0;

	FpsTimer timer;

	SpriteBatch sprites;
	GL::ShaderProgram& spriteShader = sprites.GetShader();

	SDL_Event event;
	bool running = true;
	while (running)
	{

		last = now;
		now  = SDL_GetPerformanceCounter();

		deltaTime = ((now - last) / (double)SDL_GetPerformanceFrequency());
		timer.Update(deltaTime);

		Input::PreEvent();

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) running = false;

			ImGui_ImplSDL2_ProcessEvent(&event);

			Input::HandleEvent(event);
		}

		LockMouse(Input::IsDown(Button::MouseRight));

		float mouseDeltaX = Input::GetMouseDeltaX();
		float mouseDeltaY = Input::GetMouseDeltaY();

		if (_mouseLocked)
		{
			_camera->LookDelta(mouseDeltaX * 0.25f, mouseDeltaY * 0.25f);
		}

		auto inputForce = glm::vec3(0.0f);
		if (Input::IsDown(Button::KeyW)) inputForce -= glm::vec3(0.0f, 0.0f, 1.0f);
		if (Input::IsDown(Button::KeyS)) inputForce += glm::vec3(0.0f, 0.0f, 1.0f);
		if (Input::IsDown(Button::KeyA)) inputForce += glm::vec3(1.0f, 0.0f, 0.0f);
		if (Input::IsDown(Button::KeyD)) inputForce -= glm::vec3(1.0f, 0.0f, 0.0f);
		if (glm::length2(inputForce) > 0.0f)
		{
			inputForce = glm::normalize(inputForce);
			inputForce *= Input::IsDown(Button::KeyLSHIFT) ? 60.0f : 10.0f;
			_camera->Move(inputForce, static_cast<float>(deltaTime));
		}

		_worldPhysics->Update(static_cast<float>(deltaTime));

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(static_cast<SDL_Window*>(*_window));
		ImGui::NewFrame();

		ImGui::BeginMainMenuBar();

		guiTeleportMenu();
		guiModelMenu(*_character);
		guiModelMenu(*_npcCharacter);

		ImGui::EndMainMenuBar();

		debugDrawRCF();

		if (_character != nullptr)
			_character->Update(deltaTime);

		if (_npcCharacter != nullptr)
			_npcCharacter->Update(deltaTime);

		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projectionMatrix = glm::perspective(
		    glm::radians(70.0f), io.DisplaySize.x / io.DisplaySize.y, 0.1f, 10000.0f);

		glm::mat4 viewMatrix     = _camera->GetViewMatrix();
		glm::mat4 viewProjection = projectionMatrix * viewMatrix;

		_lineRenderer->Flush(viewProjection);

		if (_level != nullptr) _level->Draw(GetResourceManager(), viewProjection);

		/*auto charPos            = _worldPhysics->GetCharacterController()->GetPosition() - glm::vec3(0.0, 0.90f, 0.0f);
		auto const& boundingBox = _skinModel->GetBoundingBox();
		auto const& boundingSphere = _skinModel->GetBoundingSphere();
		_lineRenderer->DrawAABBox(
		    boundingBox.GetMin() + charPos,
		    boundingBox.GetMax() + charPos,
		    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

		_lineRenderer->DrawSphere(
		    boundingSphere.GetCenter() + charPos,
			boundingSphere.GetRadius(), 16, 16,
		    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		*/

		// _skinShaderProgram->Bind();
		// _skinShaderProgram->SetUniformValue("viewProj", mvp);
		// _skinShaderProgram->SetUniformValue("diffuseTex", 0);
		// _skinShaderProgram->SetUniformValue("boneBuffer", 1);
		if (_character != nullptr)
			_character->Draw(viewProjection, *_skinShaderProgram, *_resourceManager);

		_npcCharacter->Draw(viewProjection, *_skinShaderProgram, *_resourceManager);

		glm::mat4 proj = glm::ortho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f);

		sprites.Begin();

		if (_textureFontP3D != nullptr)
		{
			std::string fps = fmt::format("{0} fps", timer.GetFps());
			sprites.DrawText(*_textureFontP3D, fps, glm::vec2(32 + 3, 32 + 3), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			sprites.DrawText(*_textureFontP3D, fps, glm::vec2(32, 32), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
		}

		sprites.End(proj);

		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		_window->Swap();
	}
}

void Game::guiModelMenu(Character& character)
{
	ImGui::Begin(fmt::format("Character: {0}", character.GetName()).c_str());

	if (ImGui::BeginCombo("Model", character.GetModelName().c_str()))
	{
		for (auto const& model : models)
		{
			const bool is_selected = character.GetModelName() == model.first;
			if (ImGui::Selectable(model.first.c_str(), is_selected))
			{
				character.LoadModel(model.first);
				character.LoadAnimations(model.second);
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("Animation", character.GetAnimName().c_str()))
	{
		for (auto const& anim : character.GetAnimations())
		{
			const bool is_selected = character.GetAnimName() == anim.first;
			if (ImGui::Selectable(anim.first.c_str(), is_selected))
			{
				character.SetAnimation(anim.first);
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	ImGui::InputFloat3("Position", &character.GetPosition()[0]);

	ImGui::End();
}

void Game::guiTeleportMenu()
{
	if (!ImGui::BeginMenu("Warp"))
		return;

	if (ImGui::BeginMenu("Level 1"))
	{
		for (auto const& location : locations)
		{
			if (ImGui::MenuItem(std::get<0>(location).c_str()))
			{
				const glm::vec3& dest = std::get<1>(location);
				_worldPhysics->GetCharacterController()->SetPosition(dest);
				_camera->MoveTo(dest);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(glm::to_string(std::get<1>(location)).c_str());
				ImGui::TextUnformatted(std::get<2>(location).c_str());
				ImGui::EndTooltip();
			}
		}

		ImGui::EndMenu();
	}

	ImGui::EndMenu();
}

void Game::debugDrawP3D(const P3D::P3DFile& p3d)
{
	ImGui::SetNextWindowSize(ImVec2(330, 400), ImGuiSetCond_Once);
	ImGui::Begin(p3d.GetFileName().c_str());

	ImGui::SetNextItemOpen(true); // open the root node
	const auto traverse_chunk = [&](const auto& self, const P3D::P3DChunk& chunk) -> void {
		std::ostringstream name;
		name << chunk.GetType();

		if (ImGui::TreeNode(&chunk, "%s", name.str().c_str()))
		{
			ImGui::TextDisabled("Type ID: %x", static_cast<uint32_t>(chunk.GetType()));
			ImGui::TextDisabled("Data Size: %ldb", chunk.GetData().size());			

			for (auto& child : chunk.GetChildren())
			{
				self(self, *child);
			}

			ImGui::TreePop();
		}
	};

	traverse_chunk(traverse_chunk, p3d.GetRoot());

	ImGui::End();
}

void Game::debugDrawRCF()
{
	if (_filesRCF.empty()) return;

	ImGui::SetNextWindowSize(ImVec2(330, 400), ImGuiSetCond_Once);
	ImGui::Begin("RADCORE CEMENT LIBRARY");

	for (const auto& rcf : _filesRCF)
	{
		if (ImGui::TreeNode(rcf->GetFileName().c_str()))
		{
			for (const auto& filename : rcf->GetFilenames())
			{
				if (ImGui::Selectable(filename.c_str()))
				{
					std::cout << filename.c_str() << std::endl;
					PlayAudio(*rcf, filename.c_str());
				}
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

} // namespace Donut
