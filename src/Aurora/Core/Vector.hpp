#pragma once

#include <iostream>
#include <limits>
#include "Library.hpp"

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wvolatile"
#endif

#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#include <nlohmann/json.hpp>

typedef glm::vec4 Vector4;
typedef glm::vec3 Vector3;
typedef glm::vec2 Vector2;

typedef glm::dvec4 Vector4D;
typedef glm::dvec3 Vector3D;
typedef glm::dvec2 Vector2D;

typedef glm::ivec4 Vector4i;
typedef glm::ivec3 Vector3i;
typedef glm::ivec2 Vector2i;

typedef glm::uvec4 Vector4ui;
typedef glm::uvec3 Vector3ui;
typedef glm::uvec2 Vector2ui;

typedef glm::u8vec2 Vector2B;

typedef glm::mat4 Matrix4;
typedef glm::mat3 Matrix3;

typedef glm::quat Quaternion;

namespace glm
{
	AU_API glm::dvec3 SmoothDamp(const glm::dvec3& current, glm::dvec3 target, glm::dvec3& currentVelocity, double smoothTime, double maxSpeed, double deltaTime);
	AU_API glm::dvec3 SmoothDamp(const glm::dvec3& current, const glm::dvec3& target, glm::dvec3& currentVelocity, double smoothTime, double deltaTime);
	AU_API glm::dvec3 MoveTowards(const glm::dvec3& current, const glm::dvec3& target, double maxDistanceDelta);

	// Taken from HazelDev
	AU_API bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}

namespace nlohmann
{
	template<typename T>
	struct adl_serializer<glm::vec<2, T, glm::defaultp>>
	{
		typedef glm::vec<2, T, glm::defaultp> vec;

		static_assert(
			std::is_same<T, float>::value ||
				std::is_same<T, double>::value ||
				std::is_same<T, int8_t>::value ||
				std::is_same<T, int16_t>::value ||
				std::is_same<T, int32_t>::value ||
				std::is_same<T, int64_t>::value ||
				std::is_same<T, uint8_t>::value ||
				std::is_same<T, uint16_t>::value ||
				std::is_same<T, uint32_t>::value ||
				std::is_same<T, uint64_t>::value,
			"Invalid type for vector"
		);

		static void to_json(json& j, const vec& value)
		{
			j = json::array({ value.x, value.y });
		}

		static void from_json(const json& j, vec& value)
		{
			switch(j.type())
			{
				default:
					throw std::runtime_error("Invalid value type");
				case json::value_t::number_float:
				case json::value_t::number_integer:
				case json::value_t::number_unsigned:
				{
					T v = j;
					value = vec(v, v);
					return;
				}
				case json::value_t::array:
				{
					T x = j.size() >= 1 && !j[0].empty() ? j[0].get<T>() : T();
					T y = j.size() >= 2 && !j[1].empty() ? j[1].get<T>() : T();

					value = vec(x, y);
					return;
				}
				case json::value_t::object:
				{
					if(j.empty())
					{
						value = vec(0, 0);
					}
					else
					{
						T x = j.contains("x") && !j["x"].empty() ? j["x"].get<T>() : T();
						T y = j.contains("y") && !j["y"].empty() ? j["y"].get<T>() : T();

						value = vec(x, y);
					}
					return;
				}
			}
		}
	};

	template<typename T>
	struct adl_serializer<glm::vec<3, T, glm::defaultp>>
	{
		typedef glm::vec<3, T, glm::defaultp> vec;

		static_assert(
			std::is_same<T, float>::value ||
				std::is_same<T, double>::value ||
				std::is_same<T, int8_t>::value ||
				std::is_same<T, int16_t>::value ||
				std::is_same<T, int32_t>::value ||
				std::is_same<T, int64_t>::value ||
				std::is_same<T, uint8_t>::value ||
				std::is_same<T, uint16_t>::value ||
				std::is_same<T, uint32_t>::value ||
				std::is_same<T, uint64_t>::value,
			"Invalid type for vector"
		);

		static void to_json(json& j, const vec& value)
		{
			j = json::array({ value.x, value.y, value.z });
		}

		static void from_json(const json& j, vec& value)
		{
			switch(j.type())
			{
				default:
					throw std::runtime_error("Invalid value type");
				case json::value_t::number_float:
				case json::value_t::number_integer:
				case json::value_t::number_unsigned:
				{
					T v = j;
					value = vec(v, v, v);
					return;
				}
				case json::value_t::array:
				{
					T x = j.size() > 0 && !j[0].empty() ? j[0].get<T>() : T();
					T y = j.size() > 1 && !j[1].empty() ? j[1].get<T>() : T();
					T z = j.size() > 2 && !j[2].empty() ? j[2].get<T>() : T();

					value = vec(x, y, z);
					return;
				}
				case json::value_t::object:
				{
					if(j.empty())
					{
						value = vec(0, 0, 0);
					}
					else
					{
						T x = j.contains("x") && !j["x"].empty() ? j["x"].get<T>() : T();
						T y = j.contains("y") && !j["y"].empty() ? j["y"].get<T>() : T();
						T z = j.contains("z") && !j["z"].empty() ? j["z"].get<T>() : T();

						value = vec(x, y, z);
					}
					return;
				}
			}
		}
	};

	template<typename T>
	struct adl_serializer<glm::vec<4, T, glm::defaultp>>
	{
		typedef glm::vec<4, T, glm::defaultp> vec;

		static_assert(
			std::is_same<T, float>::value ||
				std::is_same<T, double>::value ||
				std::is_same<T, int8_t>::value ||
				std::is_same<T, int16_t>::value ||
				std::is_same<T, int32_t>::value ||
				std::is_same<T, int64_t>::value ||
				std::is_same<T, uint8_t>::value ||
				std::is_same<T, uint16_t>::value ||
				std::is_same<T, uint32_t>::value ||
				std::is_same<T, uint64_t>::value,
			"Invalid type for vector"
		);

		static void to_json(json& j, const vec& value)
		{
			j = json::array({ value.x, value.y, value.z, value.w });
		}

		static void from_json(const json& j, vec& value)
		{
			switch(j.type())
			{
				default:
					throw std::runtime_error("Invalid value type");
				case json::value_t::number_float:
				case json::value_t::number_integer:
				case json::value_t::number_unsigned:
				{
					T v = j;
					value = vec(v, v, v, v);
					return;
				}
				case json::value_t::array:
				{
					T x = j.size() > 0 && !j[0].empty() ? j[0].get<T>() : T();
					T y = j.size() > 1 && !j[1].empty() ? j[1].get<T>() : T();
					T z = j.size() > 2 && !j[2].empty() ? j[2].get<T>() : T();
					T w = j.size() > 3 && !j[3].empty() ? j[3].get<T>() : T();

					value = vec(x, y, z, w);
					return;
				}
				case json::value_t::object:
				{
					if(j.empty())
					{
						value = vec(0, 0, 0, 0);
					}
					else
					{
						T x = j.contains("x") && !j["x"].empty() ? j["x"].get<T>() : T();
						T y = j.contains("y") && !j["y"].empty() ? j["y"].get<T>() : T();
						T z = j.contains("z") && !j["z"].empty() ? j["z"].get<T>() : T();
						T w = j.contains("w") && !j["w"].empty() ? j["w"].get<T>() : T();

						value = vec(x, y, z, w);
					}
					return;
				}
			}
		}
	};

}