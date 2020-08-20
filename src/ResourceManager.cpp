// Copyright 2019-2020 the donut authors. See AUTHORS.md

#include "Pure3D/Texture.h"
#include <Render/Shader.h>
#include <ResourceManager.h>

#include "Render/OpenGL/glad/gl.h"
#include "Render/imgui/imgui.h"
#include "ThirdParty/magic_enum.h"

#include <chrono>
#include <ctime>
#include <fmt/format.h>

namespace Donut
{

ResourceManager::ResourceManager()
{
	createErrorTexture();
}

ResourceManager::~ResourceManager() { }

void ResourceManager::AddTexture(std::shared_ptr<Texture> texture)
{
	// todo: move ownership out of this
	// resourcemanager is more of a map
	std::string key = texture->GetName();

	_textures[key] = std::move(texture);
	_textureCache[key] = _textures[key];

	// Clean up all expired textures (if required)
	/*_textureCache.erase(
		std::remove_if(
			_textureCache.begin(),
			_textureCache.end(),
			[](std::weak_ptr<Texture> callback) { return callback.expired(); }
		),
		_textureCache.end()
	);*/
}

void ResourceManager::createErrorTexture()
{
	// nice hot pink error texture
	constexpr GLuint errorTextureData[] = {0xFFFF00DC, 0xFF000000, 0xFF000000, 0xFFFF00DC};

	_errorTexture = std::make_shared<Texture>();
	_errorTexture->Create(2, 2, Texture::Format::RGBA8,
	                      std::vector<uint8_t>(std::begin(errorTextureData), std::end(errorTextureData)));
	_errorTexture->SetName("error");
}

void ResourceManager::ImGuiDebugWindow(bool* p_open) const
{
	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_Once);
	if (!ImGui::Begin("Resource Manager (P3D)", p_open))
	{
		ImGui::End();
		return;
	}

	const float footer_height_to_reserve =
	    ImGui::GetStyle().ItemSpacing.y + ImGui::GetTextLineHeightWithSpacing(); // 1 separator, 1 text line
	ImGui::BeginChild("##spacerfix", ImVec2(0, -footer_height_to_reserve), false);

	static ImGuiTextFilter filter;
	filter.Draw();

	ImGui::BeginTabBar("##rmtabs");

	if (ImGui::BeginTabItem("Textures"))
	{
		const float curWidth = ImGui::GetColumnWidth();

		ImGui::Columns(4, "##texturescol");
		ImGui::SetColumnWidth(0, curWidth - 96 - 96 - 96);
		ImGui::SetColumnWidth(1, 96);
		ImGui::SetColumnWidth(2, 96);
		ImGui::SetColumnWidth(3, 96);

		ImGui::Separator();
		ImGui::Text("Name");
		ImGui::NextColumn();
		ImGui::Text("Refs");
		ImGui::NextColumn();
		ImGui::Text("Weight");
		ImGui::NextColumn();
		ImGui::Text("Size");
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Columns(1);

		ImGui::BeginChild("##textures");
		ImGui::Columns(4, "##texturescol");
		ImGui::SetColumnWidth(0, curWidth - 96 - 96 - 96);
		ImGui::SetColumnWidth(1, 96);
		ImGui::SetColumnWidth(2, 96);
		ImGui::SetColumnWidth(3, 96);

		int i = 0;
		for (std::pair<std::string, std::shared_ptr<Texture>> pair : _textures)
		{
			auto name = pair.first;
			auto texture = pair.second;

			if (!filter.PassFilter(name.c_str()))
				continue;

			ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
			bool hovered = ImGui::IsItemHovered();
			if (hovered)
			{
				ImGui::BeginTooltip();
				ImGui::Image((ImTextureID)(intptr_t)texture->GetNativeTextureHandle(),
				             ImVec2((float)texture->GetWidth(), (float)texture->GetHeight()));
				ImGui::Text("Format: %s", std::string(magic_enum::enum_name(texture->GetFormat())).c_str());
				ImGui::EndTooltip();
			}

			ImGui::NextColumn();
			ImGui::Text("%d", texture.use_count());
			ImGui::NextColumn();
			ImGui::Text("%.1fKB", (float)texture->GetMemorySize() / 1024.0f);
			ImGui::NextColumn();
			ImGui::Text("%zux%zu", texture->GetWidth(), texture->GetHeight());
			ImGui::NextColumn();

			i++;
		}

		ImGui::EndChild();

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();

	ImGui::EndChild();

	ImGui::Separator();

	ImGui::TextDisabled("Textures: %zu", _textures.size());
	ImGui::SameLine();
	ImGui::TextDisabled("Shaders: %zu", _shaders.size());

	ImGui::End();
}

/*
 * Searches for the shader in the map, you should cache the result to avoid unnecessary lookups
 */
Shader* ResourceManager::GetShader(const std::string& name) const
{
	if (_shaders.find(name) == _shaders.end())
	{
		fmt::print("could not find shader {0}\n", name);
		return nullptr; // todo: return an error shader
	}

	auto const& shader = _shaders.at(name);

	// todo: check if a texture is set/valid before setting texture again
	const std::string texName = shader->GetDiffuseTextureName();
	if (_textures.find(texName) != _textures.end())
		shader->SetDiffuseTexture(_textures.at(texName).get());
	else
	{
		// fmt::print("could not find texture {1} for shader {0}\n", name, texName);
		shader->SetDiffuseTexture(_errorTexture.get());
	}

	return shader.get();
}

Texture& ResourceManager::GetTexture(const std::string& name) const
{
	if (_textures.find(name) == _textures.end())
		return *_errorTexture;

	return *_textures.at(name);
}

std::weak_ptr<Texture> ResourceManager::GetTextureWeakPtr(const std::string& name)
{
	// we return a nullptr since you probably don't want a valid reference calling this
	// if we returned an error texture you would then keep that reference, which would suck.
	// better to let the callee keep searching if there is a non existent texture..
	// todo: log this
	if (_textureCache.find(name) == _textureCache.end())
		return std::weak_ptr<Texture>();

	// are we really searching twice?
	return _textureCache.at(name);
}

Texture& ResourceManager::GetErrorTexture() const
{
	return *_errorTexture;
}

std::weak_ptr<Texture> ResourceManager::GetErrorTextureWeakPtr()
{
	return _errorTexture;
}

} // namespace Donut
