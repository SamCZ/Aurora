#pragma once
#include <cstdlib>

namespace Aurora
{
	template<typename T>
	class Stack
	{
	public:
		~Stack() { if (ptr) {  Clear();  std::free(ptr);  ptr = nullptr;  } }

		Stack()
			: size(0), capacity(32) { ptr = (T*)std::calloc(capacity, sizeof(T));  }
		Stack(int _size)
			: size(0), capacity(_size) { ptr = (T*)std::calloc(capacity, sizeof(T));  }

		// copy constructor
		Stack(const Stack& other) : size(other.size), capacity(other.capacity), ptr((T*)std::calloc(other.capacity, sizeof(T)))
		{
			for (int i = 0; i < other.size; ++i)
				ptr[i] = other.ptr[i];
		}
		// move constructor
		Stack(Stack&& other) : size(other.size), capacity(other.capacity), ptr(other.ptr)
		{
			other.ptr = nullptr;
		}

		T& operator[](int index) { return ptr[index]; }
		const T& operator[](int index) const { return ptr[index]; }
		T* begin() { return ptr; }
		T* end() { return ptr + size; }
		T* begin() const { return ptr; }
		T* end() const { return ptr + size; }
		const T* cbegin() const { return ptr; }
		const T* cend() const { return ptr + size; }
		T Top() const { return ptr[size - 1];  }
		T First() const { return *ptr; }
		inline bool Any() const { return size > 0; }
		void Clear() { size = 0; }

		void Push(T value) {
			if (size + 1 == capacity) {
				capacity += capacity / 2;
				ptr = (T*)std::realloc(ptr, capacity * sizeof(T));
			}
			ptr[size++] = value;
		}

		T Pop() { return ptr[--size]; }
		bool TryPop(T& out) {
			if (size > 0) out = ptr[--size];
			return size > 0;
		}

	public:
		int size = 0;
		int capacity = 32;
		T* ptr;
	};
}