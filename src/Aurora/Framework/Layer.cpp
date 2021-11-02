#include "Layer.hpp"

namespace Aurora
{
	std::map<LayerEnum, String> Layer::m_Layers;

	void Layer::Setup(const std::vector<String>& layerNames)
	{
		m_Layers.clear();

		au_assert(layerNames.size() <= 8);

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

		AU_LOG_WARNING("Cannot find layer by name: ", name);

		return Layer0;
	}

	String Layer::LayerToString(const LayerEnum& id)
	{
		auto it = m_Layers.find(id);

		if(it == m_Layers.end()) {
			AU_LOG_WARNING("Cannot find layer by id: ", std::to_string(id));
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