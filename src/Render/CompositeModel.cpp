#include <Render/CompositeModel.h>
#include <P3D/P3DFile.h>
#include <P3D/p3d.generated.h>
#include <iostream>

namespace Donut
{
	CompositeModel_Chunk::CompositeModel_Chunk(const P3D::P3DChunk& chunk)
	{
		for (const auto& child : chunk.GetChildren())
		{
			switch (child->GetType())
			{
			case P3D::ChunkType::CompositeDrawable:
			{
				_drawables.push_back(P3D::CompositeDrawable::Load(*child));
				break;
			}
			case P3D::ChunkType::Skeleton:
			{
				_skeletons.push_back(P3D::Skeleton::Load(*child));
				break;
			}
			case P3D::ChunkType::Mesh:
			{
				_meshes.push_back(P3D::Mesh::Load(*child));
				break;
			}
			case P3D::ChunkType::Shader:
			{
				_shaders.push_back(P3D::Shader::Load(*child));
				break;
			}
			case P3D::ChunkType::Texture:
			{
				_textures.push_back(P3D::Texture::Load(*child));
				break;
			}
			default: break;
			}
		}
	}

	CompositeModel::CompositeModel(const ICompositeModel& provider, const std::unique_ptr<ResourceManager>& resourceManager) :
		_resourceManager(resourceManager)
	{
		const auto& drawables = provider.GetDrawables();
		const auto& skeletons = provider.GetSkeletons();
		const auto& meshes = provider.GetMeshes();
		const auto& shaders = provider.GetShaders();
		const auto& textures = provider.GetTextures();

		std::map<std::string, size_t> meshNames;
		std::map<std::string, std::vector<glm::mat4>> jointTransforms;

		for (const auto& mesh : meshes)
		{
			meshNames.insert({ mesh->GetName(), _meshes.size() });
			_meshes.push_back(std::make_unique<Mesh>(*mesh));
		}

		for (const auto& skeleton : skeletons)
		{
			const auto& skeletonJoints = skeleton->GetJoints();

			std::vector<glm::mat4> transforms;
			for (const auto& joint : skeletonJoints)
			{
				transforms.push_back(transforms.empty() ? glm::mat4(1.0f) : transforms[joint->GetParent()] * joint->GetRestPose());
			}

			const auto& skeletonName = skeleton->GetName();
			jointTransforms.insert({ skeletonName, std::move(transforms) });
		}

		for (const auto& shader : shaders)
		{
			_resourceManager->AddShader(shader->GetName(), std::make_unique<Shader>(*shader));
		}

		for (const auto& texture : textures)
		{
			auto texdata = P3D::ImageData::Decode(texture->GetImage()->GetData());
			_resourceManager->AddTexture(texture->GetName(), std::move((texdata.comp == 4) ?
				std::make_unique<GL::Texture2D>(texdata.width, texdata.height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, texdata.data.data()) :
				std::make_unique<GL::Texture2D>(texdata.width, texdata.height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, texdata.data.data())));
		}

		for (const auto& drawable : drawables)
		{
			const auto& propList = drawable->GetPropList();
			if (!propList) continue;

			const auto& props = propList->GetProps();
			if (props.empty()) continue;

			const auto& transforms = jointTransforms[drawable->GetSkeletonName()];

			for (const auto& prop : props)
			{
				const auto& propName = prop->GetName();
				if (meshNames.find(propName) == meshNames.end()) continue;

				const auto& transform = transforms[prop->GetSkeletonJoint()];
				_props.push_back(DrawableProp{ meshNames[prop->GetName()], transform });
			}
		}
	}

	std::unique_ptr<CompositeModel> CompositeModel::LoadP3D(const std::string& filename, const std::unique_ptr<ResourceManager>& resourceManager)
	{
		if (!std::filesystem::exists(filename))
		{
			std::cout << "CompositeDrawable not found: " << filename << "\n";
			return nullptr;
		}

		std::cout << "Loading CompositeDrawable: " << filename << "\n";

		const auto p3d = P3D::P3DFile(filename);
		return std::make_unique<CompositeModel>(CompositeModel_Chunk(p3d.GetRoot()), resourceManager);
	}

	void CompositeModel::Draw(GL::ShaderProgram& shader, const glm::mat4& viewProj, const glm::mat4& modelMatrix)
	{
		for (const auto& prop : _props)
		{
			shader.SetUniformValue("viewProj", viewProj * modelMatrix * prop.transform);
			_meshes[prop.meshIndex]->Draw(*_resourceManager);
		}
	}
} // namespace Donut
