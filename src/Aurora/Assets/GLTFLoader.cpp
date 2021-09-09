#include "GLTFLoader.hpp"

#include "AssetManager.hpp"

#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define JSON_NOEXCEPTION
#include <tiny_gltf.h>
#include "Tools/stb_image.h"

namespace Aurora
{
	struct GLTFFileSystemCallbacks : public tinygltf::FsCallbacks
	{
		explicit GLTFFileSystemCallbacks(void* userData = nullptr) : FsCallbacks()
		{
			FileExists = &FileExistsFunction;
			ExpandFilePath = &tinygltf::ExpandFilePath;
			ReadWholeFile = &ReadWholeFileFunction;
			WriteWholeFile = &tinygltf::WriteWholeFile;
			user_data = userData;
		}

		static bool FileExistsFunction(const std::string &abs_filename, void* userData)
		{
			auto* assetManager = static_cast<AssetManager*>(userData);
			return assetManager->FileExists(abs_filename);
		}

		static bool ReadWholeFileFunction(std::vector<unsigned char>* out, std::string* err, const std::string& filepath, void* userData)
		{
			auto* assetManager = static_cast<AssetManager*>(userData);
			*out = assetManager->LoadFile(filepath);
			return !out->empty();
		}
	};

	bool LoadImageData(tinygltf::Image *image, const int image_idx, std::string *err,
	                   std::string *warn, int req_width, int req_height,
	                   const unsigned char *bytes, int size, void *user_data) {
		(void)warn;

		tinygltf::LoadImageDataOption option;
		if (user_data) {
			option = *reinterpret_cast<tinygltf::LoadImageDataOption *>(user_data);
		}

		int w = 0, h = 0, comp = 0, req_comp = 0;

		unsigned char *data = nullptr;

		// preserve_channels true: Use channels stored in the image file.
		// false: force 32-bit textures for common Vulkan compatibility. It appears
		// that some GPU drivers do not support 24-bit images for Vulkan
		req_comp = option.preserve_channels ? 0 : 4;
		int bits = 8;
		int pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

		// It is possible that the image we want to load is a 16bit per channel image
		// We are going to attempt to load it as 16bit per channel, and if it worked,
		// set the image data accodingly. We are casting the returned pointer into
		// unsigned char, because we are representing "bytes". But we are updating
		// the Image metadata to signal that this image uses 2 bytes (16bits) per
		// channel:
		if (stbi_is_16_bit_from_memory(bytes, size)) {
			data = reinterpret_cast<unsigned char *>(
					stbi_load_16_from_memory(bytes, size, &w, &h, &comp, req_comp));
			if (data) {
				bits = 16;
				pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
			}
		}

		// at this point, if data is still NULL, it means that the image wasn't
		// 16bit per channel, we are going to load it as a normal 8bit per channel
		// mage as we used to do:
		// if image cannot be decoded, ignore parsing and keep it by its path
		// don't break in this case
		// FIXME we should only enter this function if the image is embedded. If
		// image->uri references
		// an image file, it should be left as it is. Image loading should not be
		// mandatory (to support other formats)
		if (!data) data = stbi_load_from_memory(bytes, size, &w, &h, &comp, req_comp);
		if (!data) {
			// NOTE: you can use `warn` instead of `err`
			if (err) {
				(*err) +=
						"Unknown image format. STB cannot decode image data for image[" +
						std::to_string(image_idx) + "] name = \"" + image->name + "\".\n";
			}
			return false;
		}

		if ((w < 1) || (h < 1)) {
			stbi_image_free(data);
			if (err) {
				(*err) += "Invalid image data for image[" + std::to_string(image_idx) +
				          "] name = \"" + image->name + "\"\n";
			}
			return false;
		}

		if (req_width > 0) {
			if (req_width != w) {
				stbi_image_free(data);
				if (err) {
					(*err) += "Image width mismatch for image[" +
					          std::to_string(image_idx) + "] name = \"" + image->name +
					          "\"\n";
				}
				return false;
			}
		}

		if (req_height > 0) {
			if (req_height != h) {
				stbi_image_free(data);
				if (err) {
					(*err) += "Image height mismatch. for image[" +
					          std::to_string(image_idx) + "] name = \"" + image->name +
					          "\"\n";
				}
				return false;
			}
		}

		if (req_comp != 0) {
			// loaded data has `req_comp` channels(components)
			comp = req_comp;
		}

		image->width = w;
		image->height = h;
		image->component = comp;
		image->bits = bits;
		image->pixel_type = pixel_type;
		image->image.resize(static_cast<size_t>(w * h * comp) * size_t(bits / 8));
		std::copy(data, data + w * h * comp * (bits / 8), image->image.begin());
		stbi_image_free(data);

		return true;
	}

	void LoadMesh(tinygltf::Model &model, const tinygltf::Mesh& mesh)
	{
		for (size_t i = 0; i < model.bufferViews.size(); ++i)
		{
			const tinygltf::BufferView &bufferView = model.bufferViews[i];

			if (bufferView.target == 0) {  // TODO impl drawarrays
				AU_LOG_WARNING("WARN: bufferView.target is zero")
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
			std::cout << "buffer " << bufferView.buffer;

			//std::cout << "bufferview.target " << bufferView.target << std::endl;

			/*std::cout << "buffer.data.size = " << buffer.data.size()
			          << ", bufferview.byteOffset = " << bufferView.byteOffset
			          << std::endl;*/

			BufferDesc bufferDesc;

			switch (bufferView.target)
			{
				case TINYGLTF_TARGET_ARRAY_BUFFER:
					std::cout << " ARRAY_BUFFER";
					break;
				case TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER:
					std::cout << " ELEMENT_ARRAY_BUFFER";
					break;
				default:
					AU_LOG_WARNING("Unknown mesh buffer target ", bufferView.target)
					continue;
			}

			std::cout << std::endl;
		}

		for (size_t i = 0; i < mesh.primitives.size(); ++i)
		{
			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			for (auto &attrib : primitive.attributes)
			{
				tinygltf::Accessor accessor = model.accessors[attrib.second];

				int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);


			}
		}

		// TODO: Complete glTF loader
	}

	Mesh_ptr GLTFLoader::Load(const Path& path)
	{
		tinygltf::TinyGLTF loader;
		loader.SetFsCallbacks(GLTFFileSystemCallbacks(m_AssetManager));

		tinygltf::LoadImageDataOption load_image_option;
		load_image_option.preserve_channels = false;
		loader.SetImageLoader(&LoadImageData, reinterpret_cast<void *>(&load_image_option));

		std::string err;
		std::string warn;
		tinygltf::Model model;

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
			AU_LOG_ERROR("Cannot load glTF ", path)
			return nullptr;
		}

		if (!warn.empty())
		{
			AU_LOG_WARNING("glTF warning ", warn)
		}

		if (!err.empty())
		{
			AU_LOG_ERROR("glTF error ", err)
		}

		if (!res)
		{
			AU_LOG_ERROR("Failed to load glTF: ", path)
		}
		else
		{
			AU_LOG_INFO("Loaded glTF: ", path)
		}

		const tinygltf::Scene &scene = model.scenes[model.defaultScene];

		std::function<void(tinygltf::Model& model, tinygltf::Node& node)> LoadNode;
		LoadNode = [&LoadNode](tinygltf::Model& model, tinygltf::Node& node)
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

				LoadMesh(model, mesh);

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

		return nullptr;
	}
}
