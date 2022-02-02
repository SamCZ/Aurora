#pragma once

#include <cstring>
#include <cassert>
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Object.hpp"
#include "Aurora/Graphics/Color.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"

#define DEFINE_PARAM(name) static uint name
#define DECLARE_PARAM(class, name) uint class::name

namespace Aurora
{
	class AU_API MaterialBase : public ObjectBase
	{
	public:
		CLASS_OBJ(MaterialBase, ObjectBase)
	private:
		struct Parameter // NOLINT(cppcoreguidelines-pro-type-member-init)
		{
			String Name;
			String Description;
			uint Begin;
			uint End;
			uint Size;
			Texture_ptr Texture;
		};
	private:
		String m_Name;
		std::vector<uint8_t> m_ParameterBuffer;
		std::vector<Parameter> m_Parameters;
	public:
		virtual ~MaterialBase() = default;

		const String& GetName() { return m_Name; }
	protected:
		void SetName(const String& name) { m_Name = name; }
	private:
		template<typename T>
		uint CreateParam(const String& name, const T& initialValue, const String& description = "")
		{
			size_t paramSize = sizeof(T);

			Parameter param;
			param.Name = name;
			param.Description = description;
			param.Size = paramSize;
			param.Texture = nullptr;

			param.Begin = m_ParameterBuffer.size();
			param.End = param.Begin + param.Size;

			m_ParameterBuffer.resize(param.End);

			std::memcpy(m_ParameterBuffer.data() + param.Begin, &initialValue, paramSize);

			uint paramID = m_Parameters.size();
			m_Parameters.emplace_back(param);
			return paramID;
		}

		template<typename T>
		[[nodiscard]] const T& GetParamValue(uint paramID) const
		{
			assert(paramID < m_Parameters.size());
			const Parameter& param = m_Parameters[paramID];
			assert(param.Size == sizeof(T));
			const uint8_t* data = m_ParameterBuffer.data() + param.Begin;
			return reinterpret_cast<const T&>(*data);
		}

		template<typename T>
		void SetParamValue(uint paramID, const T& value)
		{
			assert(paramID < m_Parameters.size());
			const Parameter& param = m_Parameters[paramID];
			assert(param.Size == sizeof(T));
			std::memcpy(m_ParameterBuffer.data() + param.Begin, &value, param.Size);
		}
	public:
		uint CreateIntParam(const String& name, int initialValue, const String& description = "") { return CreateParam<int>(name, initialValue, description); }
		uint CreateFloatParam(const String& name, float initialValue, const String& description = "") { return CreateParam<float>(name, initialValue, description); }
		uint CreateVec2Param(const String& name, const Vector2& initialValue, const String& description = "") { return CreateParam<Vector2>(name, initialValue, description); }
		uint CreateVec3Param(const String& name, const Vector3& initialValue, const String& description = "") { return CreateParam<Vector3>(name, initialValue, description); }
		uint CreateVec4Param(const String& name, const Vector4& initialValue, const String& description = "") { return CreateParam<Vector3>(name, initialValue, description); }
		uint CreateColorParam(const String& name, Color initialValue, const String& description = "") { return CreateParam<Color>(name, initialValue, description); }

		//##########################################################################

		void GetParamInfo(uint paramID, String* name, String* description)
		{
			assert(paramID < m_Parameters.size());
			const Parameter& param = m_Parameters[paramID];

			if(name) *name = param.Name;
			if(description) *description = param.Description;
		}

		[[nodiscard]] const int& GetParamIntValue(uint paramID) const { return GetParamValue<int>(paramID); }
		[[nodiscard]] const float& GetParamFloatValue(uint paramID) const { return GetParamValue<float>(paramID); }
		[[nodiscard]] const Vector2& GetParamVec2Value(uint paramID) const { return GetParamValue<Vector2>(paramID); }
		[[nodiscard]] const Vector3& GetParamVec3Value(uint paramID) const { return GetParamValue<Vector3>(paramID); }
		[[nodiscard]] const Vector4& GetParamVec4Value(uint paramID) const { return GetParamValue<Vector4>(paramID); }
		[[nodiscard]] const Color& GetParamColorValue(uint paramID) const { return GetParamValue<Color>(paramID); }

		void SetParamIntValue(uint paramID, int val) { SetParamValue<int>(paramID, val); }
		void SetParamFloatValue(uint paramID, float val) { SetParamValue<float>(paramID, val); }
		void SetParamVec2Value(uint paramID, Vector2 val) { SetParamValue<Vector2>(paramID, val); }
		void SetParamVec3Value(uint paramID, Vector3 val) { SetParamValue<Vector3>(paramID, val); }
		void SetParamVec4Value(uint paramID, Vector4 val) { SetParamValue<Vector4>(paramID, val); }
		void SetParamColorValue(uint paramID, Color val) { SetParamValue<Color>(paramID, val); }
	public:
		uint CreateTextureParam(const String& name, const Texture_ptr& texture = nullptr, const String& description = "")
		{
			Parameter param;
			param.Name = name;
			param.Description = description;
			param.Texture = texture;
			param.Size = 0;
			param.Begin = 0;
			param.End = 0;

			uint paramID = m_Parameters.size();
			m_Parameters.emplace_back(param);
			return paramID;
		}

		[[nodiscard]] const Texture_ptr& GetParamTexture(uint paramID) const
		{
			assert(paramID < m_Parameters.size());
			const Parameter& param = m_Parameters[paramID];
			assert(param.Size == 0);
			return param.Texture;
		}

		void SetParamTexture(uint paramID, const Texture_ptr& texture)
		{
			assert(paramID < m_Parameters.size());
			Parameter& param = m_Parameters[paramID];
			assert(param.Size == 0);
			param.Texture = texture;
		}
	};
}