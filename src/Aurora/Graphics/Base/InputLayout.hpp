#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include "Format.hpp"
#include "TypeBase.hpp"

namespace Aurora
{
	struct ShaderInputVariable
	{
		std::string Name;
		size_t Size;
		GraphicsFormat Format;
		bool Instanced : 1;
		int32_t SemanticIndex;
	};

	struct VertexAttributeDesc
	{
		std::string Name;
		GraphicsFormat Format;
		uint32_t BufferIndex;
		uint32_t Offset;
		bool IsInstanced;
		uint32_t SemanticIndex;
	};

	typedef std::map<uint32_t, ShaderInputVariable> ShaderInputVariables_t;

	class IInputLayout : public TypeBase<IInputLayout>
	{
	public:
		virtual ~IInputLayout() = 0;

		virtual const std::vector<VertexAttributeDesc>& GetDescriptors() const noexcept = 0;
		virtual bool GetDescriptor(int index, VertexAttributeDesc& out_desc) const noexcept = 0;
		virtual bool GetDescriptorByName(const std::string& name, VertexAttributeDesc& out_desc) const noexcept = 0;
		virtual bool GetDescriptorBySemanticID(uint32_t semantic, VertexAttributeDesc& out_desc) const noexcept = 0;
	};

	class BasicInputLayout : public IInputLayout
	{
	private:
		std::vector<VertexAttributeDesc> m_Descriptors;
	public:
		inline BasicInputLayout(std::vector<VertexAttributeDesc> descriptors) : m_Descriptors(std::move(descriptors))
		{
		}
		~BasicInputLayout() = default;

		inline const std::vector<VertexAttributeDesc>& GetDescriptors() const noexcept override { return m_Descriptors; }
		inline bool GetDescriptor(int index, VertexAttributeDesc& out_desc) const noexcept override
		{
			if(index >= m_Descriptors.size() - 1) {
				return false;
			}

			out_desc = m_Descriptors[index];
			return true;
		}

		inline bool GetDescriptorByName(const std::string& name, VertexAttributeDesc& out_desc) const noexcept override
		{
			for(const auto& desc : m_Descriptors) {
				if(desc.Name == name) {
					out_desc = desc;
					return true;
				}
			}

			return false;
		}

		inline bool GetDescriptorBySemanticID(uint32_t semantic, VertexAttributeDesc& out_desc) const noexcept override
		{
			for(const auto& desc : m_Descriptors) {
				if(desc.SemanticIndex == semantic) {
					out_desc = desc;
					return true;
				}
			}

			return false;
		}
	};

	typedef std::shared_ptr<IInputLayout> InputLayout_ptr;
}