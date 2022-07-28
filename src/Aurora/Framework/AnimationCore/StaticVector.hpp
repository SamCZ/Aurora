#pragma once

#include <array>
#include <cassert>

template<typename T, std::size_t capacity_>
class StaticVector
{
private:
	std::array<T, capacity_> array_;
	std::size_t size_{};

public:
	static constexpr auto capacity = capacity_;

	[[nodiscard]] std::size_t size() const
	{
		return size_;
	}

	void push_back(T&& element)
	{
		assert(size_ < capacity_);
		array_[size_++] = std::move(element);
	}

	void pop_back()
	{
		assert(size_);
		--size_;
	}

	T const& operator[](std::size_t const index) const
	{
		assert(index < size_);
		return array_[index];
	}

	T& operator[](std::size_t const index)
	{
		assert(index < size_);
		return array_[index];
	}

	T const& back() const
	{
		assert(size_);
		return array_[size_ - 1];
	}

	T& back()
	{
		assert(size_);
		return array_[size_ - 1];
	}

	auto begin() const
	{
		return array_.begin();
	}

	auto begin()
	{
		return array_.begin();
	}

	auto end() const
	{
		return array_.begin() + size_;
	}

	auto end()
	{
		return array_.begin() + size_;
	}
};