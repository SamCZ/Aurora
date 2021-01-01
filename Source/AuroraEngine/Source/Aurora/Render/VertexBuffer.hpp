#pragma once

#include <Aurora/Core/Container.hpp>

namespace Aurora::Render
{
	class IVertexBuffer
	{
	public:
		virtual ~IVertexBuffer()
		{

		}

		virtual size_t GetCount() const = 0;
		virtual const void* GetData() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual uint32_t GetStride() const = 0;
		virtual void Inflate(int size) = 0;
	};

	template<typename VertexBufferType>
	class VertexBuffer : public IVertexBuffer
	{
	private:
		List<VertexBufferType> Buffer;
	public:
		virtual ~VertexBuffer() override
		{

		}

		inline void Add(const VertexBufferType& element) {
			Buffer.emplace_back(element);
		}

		inline void Emplace(VertexBufferType& element) {
			Buffer.emplace_back(element);
		}

		inline VertexBufferType& Get(int index) {
		    return Buffer[index];
		}

		inline size_t GetCount() const override {
			return Buffer.size();
		}

		inline virtual const void* GetData() const override
		{
			return &Buffer[0]; // Gets pointer to first element
		}

		inline virtual uint32_t GetSize() const override
		{
			return Buffer.size() * GetStride();
		}

		inline virtual uint32_t GetStride() const override
		{
			return sizeof(VertexBufferType);
		}

		inline void Inflate(int size) override
        {
            Buffer.reserve(Buffer.size() + size);
        }
	};
}