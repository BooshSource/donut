#include <FrontendProject.h>
#include <P3D/P3DFile.h>
#include <P3D/p3d.generated.h>
#include <Render/OpenGL/Texture2D.h>
#include <ResourceManager.h>
#include <iostream>
#include <fmt/format.h>

namespace Donut
{
	void FrontendProject::LoadP3D(const std::string& filename)
	{
		if (!std::filesystem::exists(filename))
		{
			std::cout << "Frontend Project not found: " << filename << "\n";
			return;
		}

		std::cout << "Loading Frontend Project: " << filename << "\n";

		const auto p3d = P3D::P3DFile(filename);
		const auto& root = p3d.GetRoot();
		for (const auto& chunk : root.GetChildren())
		{
			switch (chunk->GetType())
			{
				case P3D::ChunkType::FrontendProject:
				{
					auto project = P3D::FrontendProject::Load(*chunk);
					break;
				}
				case P3D::ChunkType::Sprite:
				{
					auto sprite = P3D::Sprite::Load(*chunk);
					if (sprite->GetName() != "skin-l5.png") continue;

					auto dstRow = 0;
					auto dstColumn = 0;

					auto spriteWidth = sprite->GetWidth();
					auto spriteHeight = sprite->GetHeight();
					std::vector<uint8_t> data((spriteWidth * spriteHeight) * 4);

					for (const auto& image : sprite->GetImages())
					{
						auto imageWidth = image->GetWidth();
						auto imageHeight = image->GetHeight();
						auto texdata = P3D::ImageData::Decode(image->GetData());

						for (auto row = 0; row < imageHeight - 2; ++row)
						{
							if ((dstRow + row) >= spriteHeight)
							{
								continue;
							}

							auto dstIndex = (dstRow + row) * (spriteWidth * 4);
							dstIndex += dstColumn * 4;

							auto rowDataSize = (imageWidth - 2) * 4;
							if ((spriteWidth - dstColumn) < (imageWidth - 2))
							{
								rowDataSize = (spriteWidth - dstColumn) * 4;
							}

							std::memcpy(&data[dstIndex], &texdata.data[row * (imageWidth * 4)], rowDataSize);
						}

						dstColumn += imageWidth - 2;
						if (dstColumn > spriteWidth)
						{
							dstColumn = 0;
							dstRow += imageHeight - 2;
						}
					}

					_textures.emplace_back(new GL::Texture2D(spriteWidth, spriteHeight, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data.data()));

					break;
				}
				case P3D::ChunkType::TextureFont:
				{
					auto font = P3D::TextureFont::Load(*chunk);
					break;
				}
			}
		}
	}

	void FrontendProject::Draw(const glm::mat4& proj)
	{
		float x = 0;
		float y = 0;

		for (const auto& texturePtr : _textures)
		{
			auto texture = texturePtr.get();
			_spriteBatch.Draw(texture, glm::vec2(32 + x, 70 + y), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(texture->GetWidth(), texture->GetHeight()) * 1.0f, glm::vec4(1.0f));

			x += texture->GetWidth() + 2;

			if (x > 1000)
			{
				x = 0;
				y += 256;
			}
		}

		_spriteBatch.Flush(proj);
	}
}
