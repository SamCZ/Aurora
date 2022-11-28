#pragma once

#include <vector>
#include "Types.hpp"
#include "Math.hpp"

#define DEF_OP(type) public: \
    inline friend Archive& operator<<(Archive& buffer, type value) \
    { \
        buffer.Write<type>(value); \
        return buffer; \
    } \
    \
    inline friend Archive& operator>>(Archive& buffer, type& value) \
    { \
        value = buffer.Read<type>(); \
        return buffer; \
    } \

#define DEF_OP_CAST(return_type, store_type) public: \
    inline friend Archive& operator<<(Archive& buffer, return_type value) \
    { \
        return buffer << *reinterpret_cast<const store_type*>(&value); \
    } \
    \
    inline friend Archive& operator>>(Archive& buffer, return_type& value) \
    { \
        store_type v = 0; \
        buffer >> v; \
        value = reinterpret_cast<const return_type&>(v); \
        return buffer; \
    } \

#define DEF_OP_VEC(type) public: \
    inline friend Archive& operator<<(Archive& buffer, const type& value) \
    { \
        for (int i = 0; i < type::length(); ++i) { \
            buffer << value[i]; \
        } \
        return buffer; \
    } \
    \
    inline friend Archive& operator>>(Archive& buffer, type& value) \
    { \
        for (int i = 0; i < type::length(); ++i) { \
            buffer >> value[i]; \
        } \
        return buffer; \
    } \

namespace Aurora
{
	class Archive
	{
	private:
		std::vector<uint8_t> m_Buffer;
		size_t m_CurrentRead{};
		static const std::size_t StreamEofBufferStep = 1024;
	public:
		Archive() : m_Buffer(), m_CurrentRead(0)
		{

		}

		explicit Archive(std::vector<uint8_t> buffer) : m_Buffer(std::move(buffer)), m_CurrentRead(0)
		{

		}

		explicit Archive(std::istream& in)
		{
			std::size_t size = 0;
			std::size_t size_old;
			while(true)
			{
				// Read data by `StreamEofBufferStep` segments
				size_old = size;
				size += StreamEofBufferStep;
				m_Buffer.resize(size);
				in.read(reinterpret_cast<char*>(m_Buffer.data() + size_old), StreamEofBufferStep);

				if(in.eof())
				{
					m_Buffer.resize(size_old + in.gcount()); // Discard memory to which was not written
					break;
				}
			}
		}

		Archive(std::istream& in, uint32_t byteLength, bool exceptionOnLessChars = true)
		{
			m_Buffer.resize(byteLength);
			in.read(reinterpret_cast<char*>(m_Buffer.data()), byteLength);

			m_Buffer.resize(in.gcount());

			if(in.eof() && exceptionOnLessChars)
				throw std::runtime_error("Stream ended too soon");
		}

		Archive(const uint8_t* array, std::size_t arraySize)
		{
			/*if(arraySize > MaxSize)
				throw std::runtime_error("Source byte array is too big");*/

			m_Buffer.resize(arraySize);
			std::copy(array, array + arraySize, m_Buffer.data());
		}

		template<typename T>
		inline void Write(T data)
		{
			m_Buffer.reserve(sizeof(T));

			for(std::size_t i = 0; i < sizeof(T); i++)
				m_Buffer.emplace_back(*(reinterpret_cast<char*>(&data) + i));
		}

		template<typename T>
		inline T Read()
		{
			T var;

			memcpy(&var, GetBufferPointer() + m_CurrentRead, sizeof(T));
			m_CurrentRead += sizeof(T);

			return var;
		}

		template<typename T>
		inline void WriteArray(const T* data, size_t size)
		{
			for (size_t i = 0; i < size; ++i) {
				(*this) << data[i];
			}
		}

		template<typename T>
		inline void ReadArray(T* data, size_t size)
		{
			for (size_t i = 0; i < size; ++i) {
				(*this) >> data[i];
			}
		}

		inline friend std::ostream& operator<<(std::ostream& out, Archive& buffer)
		{
			out.write(reinterpret_cast<const char*>(buffer.GetBufferPointer()), buffer.GetLength());

			return out;
		}

		inline uint8_t* GetBufferPointer() { return GetBuffer().data(); }
		inline std::vector<uint8_t>& GetBuffer() { return m_Buffer; }
		inline size_t GetSize() { return m_Buffer.size() * sizeof(uint8_t); }
		inline size_t GetLength() { return m_Buffer.size(); }
	public:
		DEF_OP(int8_t);
		DEF_OP(uint8_t);

		DEF_OP(int16_t);
		DEF_OP(uint16_t);

		DEF_OP(int32_t);
		DEF_OP(uint32_t);

		DEF_OP(int64_t);
		DEF_OP(uint64_t);

		DEF_OP_CAST(float, uint32_t);
		DEF_OP_CAST(double, uint64_t);
		DEF_OP_CAST(bool, uint8_t);

		DEF_OP_VEC(Vector2);
		DEF_OP_VEC(Vector2i);
		DEF_OP_VEC(Vector2D);
		DEF_OP_VEC(Vector3);
		DEF_OP_VEC(Vector3i);
		DEF_OP_VEC(Vector3D);
		DEF_OP_VEC(Vector4);
		DEF_OP_VEC(Vector4i);
		DEF_OP_VEC(Vector4D);

		DEF_OP_VEC(Matrix3);
		DEF_OP_VEC(Matrix4);
		DEF_OP_VEC(Quaternion);
	public:
		inline friend Archive& operator<<(Archive& buffer, const std::string& value)
		{
			buffer << static_cast<int>(value.length());

			for (char i : value) {
				buffer << static_cast<int8_t>(i);
			}

			return buffer;
		}

		inline friend Archive& operator>>(Archive& buffer, std::string& value)
		{
			int len;
			buffer >> len;
			for (int i = 0; i < len; ++i) {
				int8_t c;
				buffer >> c;
				value += c;
			}
			return buffer;
		}

		template<typename T>
		inline friend Archive& operator<<(Archive& buffer, const std::vector<T>& list)
		{
			buffer << static_cast<uint32_t>(list.size());
			for(auto& e : list) {
				buffer.Write<T>(e);
			}
			return buffer;
		}

		template<typename T>
		inline friend Archive& operator>>(Archive& buffer, std::vector<T>& list)
		{
			list.clear();
			uint32_t count;
			buffer >> count;
			for (int i = 0; i < count; ++i) {
				list.push_back(buffer.Read<T>());
			}
			return buffer;
		}
	};
}
#undef DEF_OP
#undef DEF_OP_CAST
#undef DEF_OP_VEC