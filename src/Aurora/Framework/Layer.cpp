#include "Layer.hpp"
#include <cassert>

namespace Aurora
{
	std::map<LayerEnum, String> Layer::m_Layers;

	void Layer::Setup(const std::vector<String>& layerNames)
	{
		m_Layers.clear();

		assert(layerNames.size() < sizeof(uint32_t) * 8);

		for (uint32_t i = 0; i < layerNames.size(); ++i) {
			m_Layers[(LayerEnum)i] = layerNames[i];
		}
	}

	LayerEnum Layer::NameToLayer(const String &name)
	{
		for(auto& it : m_Layers) {
			if(it.second == name) {
				return it.first;
			}
		}

#ifdef DEBUG
		throw std::runtime_error("Cannot find layer by name: " + name);
#endif

		return Layer0;
	}

	String Layer::LayerToString(const LayerEnum& id)
	{
		auto it = m_Layers.find(id);

		if(it == m_Layers.end()) {
#ifdef DEBUG
			throw std::runtime_error("Cannot find layer by id: " + std::to_string(id));
#endif
			return "Unknown";
		}

		return it->second;
	}

	const std::map<LayerEnum, String>& Layer::GetLayers()
	{
		return m_Layers;
	}

	size_t Layer::GetLayerCount()
	{
		return m_Layers.size();
	}
}