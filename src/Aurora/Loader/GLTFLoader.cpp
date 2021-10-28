#include "GLTFLoader.hpp"

#include <nlohmann/json.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_NO_INCLUDE_JSON
#include <tiny_gltf.h>

#include <Aurora/Core/Common.hpp>
#include <Aurora/Graphics/Mesh.hpp>
#include <Aurora/Graphics/Base/IRenderDevice.hpp>
#include <Aurora/Graphics/Material/MaterialPBR.hpp>

namespace Aurora
{

	Texture_ptr GLTFImageToTexture(tinygltf::Image& image, IRenderDevice* renderDevice)
	{
		TextureDesc desc;
		desc.Width = image.width;
		desc.Height = image.height;
		desc.Name = image.name;
		desc.MipLevels = desc.GetMipLevelCount();

		switch(image.component)
		{
			case 1:
			{
				switch(image.bits)
				{
					case 8:
						desc.ImageFormat = GraphicsFormat::R8_UNORM;
						break;
					default:
						AU_LOG_FATAL("Image bit format not supported ! ", image.name, " - ", image.bits);
				}
				break;
			}
			case 2:
			{
				switch(image.bits)
				{
					case 8:
						desc.ImageFormat = GraphicsFormat::RG8_UNORM;
						break;
					default:
						AU_LOG_FATAL("Image bit format not supported ! ", image.name, " - ", image.bits);
				}
				break;
			}
			case 3:
			{
				AU_LOG_FATAL("Image component format not supported ! ", image.name, " - ", image.component);
				break;
			}
			case 4:
			{
				switch(image.bits)
				{
					case 8:
						desc.ImageFormat = GraphicsFormat::SRGBA8_UNORM;
						break;
					default:
						AU_LOG_FATAL("Image bit format not supported ! ", image.name, " - ", image.bits);
				}
				break;
			}
		}

		auto texture = renderDevice->CreateTexture(desc);
		renderDevice->WriteTexture(texture, 0, 0, &image.image.at(0));
		renderDevice->GenerateMipmaps(texture);

		return texture;
	}

	XMesh* LoadMesh(tinygltf::Model &model, const tinygltf::Mesh& mesh, IRenderDevice* renderDevice)
	{
		std::vector<Buffer_ptr> buffers;

		for (size_t i = 0; i < model.bufferViews.size(); ++i)
		{
			const tinygltf::BufferView &bufferView = model.bufferViews[i];

			if (bufferView.target == 0) {  // TODO impl drawarrays
				AU_LOG_WARNING("WARN: bufferView.target is zero");
				continue;  // Unsupported bufferView.
				/*
				  From spec2.0 readme:
				  https://github.com/KhronosGroup/glTF/tree/master/specification/2.0
						   ... drawArrays function should be used with a count equal to
				  the count            property of any of the accessors referenced by the
				  attributes            property            (they are all equal for a given
				  primitive).
				*/
			}

			const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
			//std::cout << "buffer " << bufferView.name << i;

			//std::cout << "bufferview.target " << bufferView.target << std::endl;

			/*std::cout << "buffer.data.size = " << buffer.data.size()
					  << ", bufferview.byteOffset = " << bufferView.byteOffset
					  << std::endl;*/

			BufferDesc bufferDesc;
			bufferDesc.ByteSize = bufferView.byteLength;
			bufferDesc.Usage = EBufferUsage::StaticDraw;

			switch (bufferView.target)
			{
				case TINYGLTF_TARGET_ARRAY_BUFFER:
					//std::cout << " ARRAY_BUFFER";
					bufferDesc.Name = "GLTF VB " + std::to_string(i);
					bufferDesc.Type = EBufferType::VertexBuffer;
					break;
				case TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER:
					//std::cout << " ELEMENT_ARRAY_BUFFER";
					bufferDesc.Name = "GLTF IB " + std::to_string(i);
					bufferDesc.Type = EBufferType::IndexBuffer;
					break;
				default:
					AU_LOG_WARNING("Unknown mesh buffer target ", bufferView.target);
					continue;
			}

			auto gpuBuffer = renderDevice->CreateBuffer(bufferDesc);
			renderDevice->WriteBuffer(gpuBuffer, &buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength, 0);
			buffers.emplace_back(gpuBuffer);

			//std::cout << " OFFSET(" << bufferView.byteOffset << ")";
			//std::cout << std::endl;
		}

		std::cout << "VB count " << buffers.size() <<std::endl;

		auto* gpuMesh = new XMesh();
		gpuMesh->m_Buffers = buffers;
		gpuMesh->m_Sections.clear();

		for (size_t i = 0; i < mesh.primitives.size(); ++i)
		{
			tinygltf::Primitive primitive = mesh.primitives[i];

			std::vector<VertexAttributeDesc> vertexAttrs;
			int attrId = 0;
			for (auto &attrib : primitive.attributes)
			{
				tinygltf::Accessor accessor = model.accessors[attrib.second];

				int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
				VertexAttributeDesc attributeDesc;
				attributeDesc.Name = attrib.first;
				attributeDesc.BufferIndex = accessor.bufferView;
				attributeDesc.Offset = accessor.byteOffset;
				attributeDesc.IsInstanced = false;
				attributeDesc.Normalized = accessor.normalized;
				attributeDesc.Format = GraphicsFormat::Unknown;
				attributeDesc.Stride = byteStride;
				attributeDesc.SemanticIndex = attrId;
				attrId++;

				if(attributeDesc.Name == "POSITION")
				{
					Buffer_ptr buffer = buffers[attributeDesc.BufferIndex];

					auto* vertices = renderDevice->MapBuffer<Vector3>(buffer, EBufferAccess::ReadOnly);
					{

					}
					renderDevice->UnmapBuffer(buffer);
				}

				//std::cout << attrib.first;

				switch (accessor.type)
				{
					case TINYGLTF_TYPE_VEC2:
						attributeDesc.Format = GraphicsFormat::RG32_FLOAT;
						//std::cout << " TINYGLTF_TYPE_VEC2";
						break;
					case TINYGLTF_TYPE_VEC3:
						attributeDesc.Format = GraphicsFormat::RGB32_FLOAT;
						//std::cout << " TINYGLTF_TYPE_VEC3";
						break;
					case TINYGLTF_TYPE_VEC4:
						attributeDesc.Format = GraphicsFormat::RGBA32_FLOAT;
						//std::cout << " TINYGLTF_TYPE_VEC4";
						break;
					default:
						AU_LOG_FATAL("Known mesh attr format");
						break;
				}

				//std::cout << std::endl;

				vertexAttrs.emplace_back(attributeDesc);

				//
			}

			if(primitive.indices < 0) continue;
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			assert(!indexAccessor.sparse.isSparse);

			XMesh::PrimitiveSection section;
			section.Layout = renderDevice->CreateInputLayout(vertexAttrs);
			section.BufferIndex = indexAccessor.bufferView;
			section.AddRange(indexAccessor.count, indexAccessor.byteOffset);

			switch (indexAccessor.componentType)
			{
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					section.IndexFormat = EIndexBufferFormat::Uint32;
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					section.IndexFormat = EIndexBufferFormat::Uint16;
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					section.IndexFormat = EIndexBufferFormat::Uint8;
					break;
				default:
					AU_LOG_FATAL("Unsupported index format in GLTF mesh !");
					break;
			}

			switch (primitive.mode)
			{
				case TINYGLTF_MODE_POINTS:
					section.PrimitiveType = EPrimitiveType::PointList;
					break;
				case TINYGLTF_MODE_TRIANGLES:
					section.PrimitiveType = EPrimitiveType::TriangleList;
					break;
				case TINYGLTF_MODE_TRIANGLE_STRIP:
					section.PrimitiveType = EPrimitiveType::TriangleStrip;
					break;
				default:
					AU_LOG_FATAL("Unsupported primitive mode !");
					break;
			}


			// Load material
			const tinygltf::Material& material = model.materials[primitive.material];

			matref gameMaterial = std::make_shared<MaterialPBR>();

			for(const auto& it : material.values)
			{
				//std::cout << it.first << " - " << it.second.TextureIndex() << std::endl;
			}

			for(const auto& it : material.additionalValues)
			{
				//std::cout << it.first << " - " << it.second.string_value << std::endl;
			}

			tinygltf::PbrMetallicRoughness pbrData = material.pbrMetallicRoughness;

			Vector3 baseColor = {pbrData.baseColorFactor[0], pbrData.baseColorFactor[1], pbrData.baseColorFactor[2]};

			gameMaterial->SetParamVec3Value(MaterialPBR::MP_BASE_COLOR, baseColor);

			auto baseColorParam = material.values.find("baseColorTexture");
			if(baseColorParam != material.values.end())
			{
				tinygltf::Texture& texture = model.textures[baseColorParam->second.TextureIndex()];
				tinygltf::Image &image = model.images[texture.source];

				//section.BaseColor = GLTFImageToTexture(image, renderDevice);
				gameMaterial->SetParamTexture(MaterialPBR::MP_ALBEDO_MAP, GLTFImageToTexture(image, renderDevice));
			}

			if(material.normalTexture.index >= 0)
			{
				tinygltf::Texture& texture = model.textures[material.normalTexture.index];
				tinygltf::Image &image = model.images[texture.source];
				//section.NormalMap = GLTFImageToTexture(image, renderDevice);
				gameMaterial->SetParamTexture(MaterialPBR::MP_NORMAL_MAP, GLTFImageToTexture(image, renderDevice));
			}

			gpuMesh->Materials->SetMaterial(i, gameMaterial);

			section.MaterialIndex = i; // TODO
			// indexes

			gpuMesh->m_Sections.emplace_back(section);
		}

		// TODO: Complete glTF loader

		return gpuMesh;
	}

	std::vector<std::shared_ptr<XMesh>> GLTFLoader::LoadMeshFile(const Path &path, IRenderDevice* renderDevice)
	{
		tinygltf::TinyGLTF loader;

		std::string err;
		std::string warn;
		tinygltf::Model model;

		std::vector<std::shared_ptr<XMesh>> meshes;

		bool res;
		Path extension = path.extension();
		if (extension == ".glb")
		{
			res = loader.LoadBinaryFromFile(&model, &err, &warn, path.string());
		}
		else if (extension == ".gltf")
		{
			res = loader.LoadASCIIFromFile(&model, &err, &warn, path.string());
		}
		else
		{
			AU_LOG_ERROR("Cannot load glTF ", path);
			return meshes;
		}

		if (!warn.empty())
		{
			AU_LOG_WARNING("glTF warning ", warn);
		}

		if (!err.empty())
		{
			AU_LOG_ERROR("glTF error ", err);
		}

		if (!res)
		{
			AU_LOG_ERROR("Failed to load glTF: ", path);
		}
		else
		{
			AU_LOG_INFO("Loaded glTF: ", path);
		}

		const tinygltf::Scene &scene = model.scenes[model.defaultScene];

		std::function<void(tinygltf::Model& model, tinygltf::Node& node)> LoadNode;
		LoadNode = [&LoadNode, &meshes, renderDevice](tinygltf::Model& model, tinygltf::Node& node)
		{
			std::cout << node.name << std::endl;

			auto transform = glm::identity<Matrix4>();

			if(!node.translation.empty())
			{
				transform *= glm::translate(Vector3(node.translation[0], node.translation[1], node.translation[2]));
				std::cout << "transform: x=" << node.translation[0] << ", y=" << node.translation[1] << ", z=" << node.translation[2] << std::endl;
			}

			if(!node.rotation.empty())
			{
				Quaternion quaternion(static_cast<float>(node.rotation[3]), static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2]));
				transform *= glm::toMat4(quaternion);
				std::cout << "rotation: " << glm::to_string(quaternion) << std::endl;
			}

			if(!node.scale.empty())
			{
				transform *= glm::scale(Vector3(node.scale[0], node.scale[1], node.scale[2]));
				std::cout << "scale: x=" << node.scale[0] << ", y=" << node.scale[1] << ", z=" << node.scale[2] << std::endl;
			}

			if ((node.mesh >= 0) && (node.mesh < model.meshes.size()))
			{
				const tinygltf::Mesh& mesh = model.meshes[node.mesh];

				auto gpuMesh = LoadMesh(model, mesh, renderDevice);
				gpuMesh->m_BaseTransform = transform;

				if(gpuMesh)
				{
					meshes.push_back(std::shared_ptr<XMesh>(gpuMesh));
				}

				std::cout << " - Mesh " << mesh.name << std::endl;
			}

			for (size_t i = 0; i < node.children.size(); i++)
			{
				assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
				LoadNode(model, model.nodes[node.children[i]]);
			}
		};

		for (size_t i = 0; i < scene.nodes.size(); ++i)
		{
			assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));

			LoadNode(model, model.nodes[scene.nodes[i]]);
		}

		return meshes;
	}
}