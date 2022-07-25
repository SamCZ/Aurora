#pragma once
#include <utility>
#include <cassert>
#include <xmemory>

// you may don't want to use constexpr because of compile times.
#define HUSTLE_USE_CONSTEXPR

#ifdef HUSTLE_USE_CONSTEXPR
#	define HSCONSTEXPR constexpr
#else
#	define HSCONSTEXPR 
#endif

namespace Aurora
{
	using uint8  = unsigned char;
	using int8   = char;
	using uint   = unsigned int;
	using ulong  = unsigned long;

	namespace Hasher
	{
		template <typename T> ulong Hash(const T& val)
		{
			static_assert("you must define custom hash function");
			return 0;
		}

		template <> ulong Hash(const float& f)
		{
			union Converter { float fVal; ulong uval; } converter;
			converter.fVal = f;
			return converter.uval;
		}

		template <> ulong Hash(const int& in)
		{
			return (ulong)in;
		}
	}

	template<typename T>
	constexpr void MemCpy(T* a, const T* b, int count)
	{
		while (count--) a[count] = b[count];
	}

	template<typename T>
	constexpr void MemMove(T* a, T* b, int count)
	{
		while (count--) a[count] = std::move(b[count]);
	}

	// const size, non growable
	// all operations is O(1), and memory allocation, initialization is also fast,
	// chace friendly stack allocated contigues memory
	template<typename Key, typename Value, int Size = 90, int8 BucketSize = 32>
	class StaticHashMap
	{
	private:
		int8 numElements[Size] { 0 };
		Key keys[Size * BucketSize] { };
		mutable Value arr[Size * BucketSize] { };
	public:

		HSCONSTEXPR StaticHashMap()  { }

		~StaticHashMap()  { }

		HSCONSTEXPR StaticHashMap(const StaticHashMap& other) // copy constructor
		{
			MemCpy(numElements, other.numElements, BucketSize);
			MemCpy(arr, other.arr, Size * BucketSize);
		}

		HSCONSTEXPR StaticHashMap(const StaticHashMap&& other) // move constructor, we cant move just copy
		{
			MemMove(numElements, other.numElements, BucketSize);
			MemMove(arr, other.arr, Size * BucketSize);
		}

	public:

		HSCONSTEXPR inline void Insert(const Key& key, Value&& value)
		{
			uint bucketIndex = Hasher::Hash(key) % Size;
			uint arrIndex = bucketIndex * BucketSize + (numElements[bucketIndex]++);
			assert(numElements[bucketIndex] <= BucketSize); // BucketSize is small
			assert(arrIndex < (Size * BucketSize)); // Size is small
			keys[arrIndex] = key;
			arr[arrIndex] = value;
		}

		template<typename... T>
		HSCONSTEXPR inline void Emplace(const Key& key, T&&... values)
		{
			uint bucketIndex = Hasher::Hash(key) % Size;
			uint arrIndex = bucketIndex * BucketSize + (numElements[bucketIndex]++);
			assert(numElements[bucketIndex] <= BucketSize); // BucketSize is small
			assert(arrIndex < (Size * BucketSize)); // Size is small

			keys[arrIndex] = key;
			arr[arrIndex] = Value(std::forward<T>(values)...);
		}

		// returns true if removed correctly
		HSCONSTEXPR bool Remove(const Key& key)
		{
			const uint bucketIndex = Hasher::Hash(key) % Size;
			const uint  valueIndex = bucketIndex * BucketSize;
			const uint8 bucketSize = numElements[bucketIndex];

			for (uint8 i = 0; i < bucketSize; ++i)
			{
				if (keys[valueIndex + i] == key)
				{
					arr[valueIndex + i] = arr[--numElements[bucketIndex]];
					return true;
				}
			}
			return false;
		}

		HSCONSTEXPR bool Contains(const Key& key) const
		{
			const uint bucketIndex = Hasher::Hash(key) % Size;
			const uint  valueIndex = bucketIndex * BucketSize;
			const uint8 bucketSize = numElements[bucketIndex];

			for (uint8 i = 0; i < bucketSize; ++i)
				if (keys[valueIndex + i] == key) return true;

			return false;
		}

		HSCONSTEXPR Value* Find(const Key& key)
		{
			const uint bucketIndex = Hasher::Hash(key) % Size;
			const uint  valueIndex = bucketIndex * BucketSize;
			uint8 currBucketIndex = numElements[bucketIndex];

			while (currBucketIndex--)
			{
				if (keys[valueIndex + currBucketIndex] == key) return &arr[valueIndex + currBucketIndex];
			}
			return nullptr;
		}

		HSCONSTEXPR Value* FindOrCreate(const Key& key)
		{
			const uint bucketIndex = Hasher::Hash(key) % Size;
			uint  valueIndex = bucketIndex * BucketSize;
			uint8 currBucketIndex = numElements[bucketIndex];
			while (currBucketIndex--)
			{
				if (keys[valueIndex + currBucketIndex] == key) return &arr[valueIndex + currBucketIndex];
			}

			valueIndex += numElements[bucketIndex]++;

			assert(numElements[bucketIndex] <= BucketSize); // BucketSize is small
			assert(valueIndex  < (Size * BucketSize)); // Size is small

			keys[valueIndex] = key;
			arr[valueIndex] = Value();
			return &arr[valueIndex] ;
		}

		HSCONSTEXPR inline bool Empty() const
		{
			int currentBucket = Size;
			while (currentBucket--) if (numElements[currentBucket]) return true;
			return false;
		}

		HSCONSTEXPR int8 Count(const Key& key) const
		{
			const uint bucketIndex = Hasher::Hash(key) % Size;
			const uint  valueIndex = bucketIndex * BucketSize;
			const uint8 bucketSize = numElements[bucketIndex];
			int8 count = 0;

			for (uint8 i = 0; i < bucketSize; ++i)
				count += keys[valueIndex + i] == key;

			return count;
		}

		[[maybe_unused]] HSCONSTEXPR int8 CountBucket(const Key& key) const
		{
			return numElements[Hasher::Hash(key) % Size];
		}

		template<typename Callable_t>
		HSCONSTEXPR void Iterate(const Callable_t& func)
		{
			int currentBucket = Size;
			while (currentBucket--)
			{
				for (int i = 0; i < numElements[currentBucket]; ++i)
				{
					func(arr[currentBucket * BucketSize + i]);
				}
			}
		}

		HSCONSTEXPR void Clear()
		{
			int currentBucket = Size;
			while (currentBucket--)
			{
				while (numElements[currentBucket]--)
				{
					arr[currentBucket * BucketSize + numElements[currentBucket]].~Value();
				}
				numElements[currentBucket] = 0;
			}
		}

		// cleans all of the used instances and recreates them
		HSCONSTEXPR void Reload()
		{
			for(int currentBucket = 0; currentBucket < Size; ++currentBucket)
			{
				while (numElements[currentBucket]--)
				{
					int index = currentBucket * BucketSize + numElements[currentBucket];
					arr[index].~Value();
					arr[index] = Value();
				}
				numElements[currentBucket] = 0;
			}
		}

		// calls default constructor for each element
		HSCONSTEXPR inline void Initialize()
		{
			for (int i = 0; i < Size * BucketSize; ++i)
			{
				arr[i].~Value();
				arr[i] = Value();
			}
		}

		HSCONSTEXPR inline Value& operator[](const Key& key) const
		{
			return *Find(key);
		}

		class Iterator
		{
		public:
			int currBucketIndex = 0;
			int currIndex = 0;
			const StaticHashMap* hashMap;
		public:
			HSCONSTEXPR Iterator(const StaticHashMap* map, int cBucketIndex, int cIndex)
				: hashMap(map), currBucketIndex(cBucketIndex), currIndex(cIndex) { }

			HSCONSTEXPR Value& operator*() const
			{
				return hashMap->arr[currBucketIndex * BucketSize + currIndex];
			}

			// prefix increment
			HSCONSTEXPR Iterator& operator++()
			{
				currBucketIndex += ++currIndex >= hashMap->numElements[currBucketIndex];
				currIndex *= currIndex < hashMap->numElements[currBucketIndex];
				return *this;
			}

			// postfix increment
			HSCONSTEXPR Iterator operator++(int amount)
			{
				currBucketIndex += ++currIndex >= hashMap->numElements[currBucketIndex];
				currIndex *= currIndex < hashMap->numElements[currBucketIndex];
				return *this;
			}

			HSCONSTEXPR Value* operator->()
			{
				return &hashMap->arr[currBucketIndex * BucketSize + currIndex];
			}

			HSCONSTEXPR bool operator == (const Iterator& other) const
			{
				return currBucketIndex == other.currBucketIndex && currIndex == other.currIndex;
			}

			HSCONSTEXPR bool operator != (const Iterator& other) const
			{
				return currBucketIndex != other.currBucketIndex || currIndex != other.currIndex;
			}

			HSCONSTEXPR bool operator < (const Iterator& other) const
			{
				return currBucketIndex * BucketSize + currIndex < other.currBucketIndex * BucketSize + other.currIndex;
			}

			HSCONSTEXPR bool operator > (const Iterator& other) const
			{
				return currBucketIndex * BucketSize + currIndex > other.currBucketIndex * BucketSize + other.currIndex;
			}
		};

		class ConstIterator
		{
		public:
			int currBucketIndex = 0;
			mutable int currIndex = 0;
			const StaticHashMap* hashMap;
		public:
			HSCONSTEXPR ConstIterator(const StaticHashMap* map, int cBucketIndex, int cIndex)
				: hashMap(map), currBucketIndex(cBucketIndex), currIndex(cIndex) { }

			HSCONSTEXPR const Value& operator*() const
			{
				return hashMap->arr[currBucketIndex * BucketSize + currIndex];
			}

			// prefix increment
			HSCONSTEXPR const ConstIterator& operator++() const
			{
				currBucketIndex += ++currIndex >= hashMap->numElements[currBucketIndex];
				currIndex *= currIndex < hashMap->numElements[currBucketIndex];
				return *this;
			}

			// postfix increment
			HSCONSTEXPR const ConstIterator operator++(int amount) const
			{
				currBucketIndex += ++currIndex >= hashMap->numElements[currBucketIndex];
				currIndex *= currIndex < hashMap->numElements[currBucketIndex];
				return *this;
			}

			HSCONSTEXPR const Value* operator->() const
			{
				return &hashMap->arr[currBucketIndex * BucketSize + currIndex];
			}

			HSCONSTEXPR bool operator == (const ConstIterator& other) const
			{
				return currBucketIndex == other.currBucketIndex && currIndex == other.currIndex;
			}
			HSCONSTEXPR bool operator != (const ConstIterator& other) const
			{
				return currBucketIndex != other.currBucketIndex || currIndex != other.currIndex;
			}
			HSCONSTEXPR bool operator < (const ConstIterator& other) const
			{
				return currBucketIndex * BucketSize + currIndex < other.currBucketIndex * BucketSize + other.currIndex;
			}
			HSCONSTEXPR bool operator > (const ConstIterator& other) const
			{
				return currBucketIndex * BucketSize + currIndex > other.currBucketIndex * BucketSize + other.currIndex;
			}
		};

		HSCONSTEXPR Iterator begin() const { return Iterator(this, 0, 0); }
		HSCONSTEXPR Iterator end()   const { return Iterator(this, Size-1, BucketSize-1); }

		HSCONSTEXPR ConstIterator cbegin() const { return Iterator(this, 0, 0); }
		HSCONSTEXPR ConstIterator cend()   const { return Iterator(this, Size-1, BucketSize-1); }

	};

	template<typename Key, typename Value, int Size = 61, int8 BucketSize = 4>
	class StaticSet : public StaticHashMap<Key, Value, Size, BucketSize>
	{
		
	};
}