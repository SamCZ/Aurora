#pragma once

#include <map>
#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"

namespace Aurora
{
	enum LayerEnum : uint16_t
	{
		Layer0 = 0,
		Layer1,
		Layer2,
		Layer3,
		Layer4,
		Layer5,
		Layer6,
		Layer7,
		Layer8,
		Layer9,
		NumLayers
	};

	class Layer
	{
	public:
		typedef uint16_t Hash_t;
	private:
		static std::map<LayerEnum, String> m_Layers;
		Hash_t m_Hash;
	public:
		inline Layer() : m_Hash()
		{
			(*this).operator=(Layer0);
		}

		explicit inline Layer(const LayerEnum& layer) : m_Hash()
		{
			(*this).operator=(layer);
		}

		explicit inline Layer(const String& layer) : m_Hash()
		{
			(*this).operator=(layer);
		}
	public:
		static void Setup(const std::vector<String>& layerNames);
		static LayerEnum NameToLayer(const String& name);
		static String LayerToString(const LayerEnum& id);

		static const std::map<LayerEnum, String> & GetLayers();
		static size_t GetLayerCount();

		inline Layer& operator=(const LayerEnum& layerEnum)
		{
			m_Hash = 1u << layerEnum;
			return *this;
		}

		inline Layer& operator=(const String& layerEnum)
		{
			return (*this).operator=(NameToLayer(layerEnum));
		}

		inline Layer& operator+=(const LayerEnum& layerEnum)
		{
			m_Hash |= 1u << layerEnum;
			return *this;
		}

		inline Layer& operator+=(const String& layerEnum)
		{
			return (*this).operator+=(NameToLayer(layerEnum));
		}

		inline Layer& operator-=(const LayerEnum& layerEnum)
		{
			m_Hash &= ~(1u << layerEnum);
			return *this;
		}

		inline Layer& operator-=(const String& layerEnum)
		{
			return (*this).operator-=(NameToLayer(layerEnum));
		}

		inline bool operator&(const LayerEnum& layerEnum) const
		{
			return (m_Hash & (1u << layerEnum)) != 0;
		}

		inline Hash_t Hash() const
		{
			return m_Hash;
		}
	};
}
