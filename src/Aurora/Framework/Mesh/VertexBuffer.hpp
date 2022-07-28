#pragma once

#include "Aurora/Core/Types.hpp"

namespace Aurora
{
	class IVertexBuffer
	{
	public:
		virtual ~IVertexBuffer() = default;

		[[nodiscard]] virtual size_t GetCount() const = 0;
		[[nodiscard]] virtual const uint8* GetData() const = 0;
		[[nodiscard]] virtual uint32_t GetSize() const = 0;
		[[nodiscard]] virtual uint32_t GetStride() const = 0;
		virtual void Inflate(int size) = 0;
		virtual void Clear() = 0;
	};

	template<typename VertexBufferType>
	class VertexBuffer : public IVertexBuffer
	{
	private:
		std::vector<VertexBufferType> Buffer{};
	public:
		VertexBuffer() = default;
		explicit VertexBuffer(uint count)
		{
			Buffer.reserve(count);
		}
		~VertexBuffer() override = default;

		inline void Add(const VertexBufferType& element) {
			Buffer.emplace_back(element);
		}

		inline void Emplace(VertexBufferType& element) {
			Buffer.emplace_back(element);
		}

		inline VertexBufferType& Get(unsigned int index) {
			return Buffer[index];
		}

		[[nodiscard]] inline size_t GetCount() const override {
			return Buffer.size();
		}

		[[nodiscard]] inline const uint8* GetData() const override
		{
			return reinterpret_cast<const uint8*>(Buffer.data()); // Gets pointer to first element
		}

		[[nodiscard]] inline uint32_t GetSize() const override
		{
			return Buffer.size() * GetStride();
		}

		[[nodiscard]] inline uint32_t GetStride() const override
		{
			return sizeof(VertexBufferType);
		}

		inline void Inflate(int size) override
		{
			Buffer.reserve(Buffer.size() + size);
		}

		void Clear() override
		{
			Buffer.clear();
		}
	};
}